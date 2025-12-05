//
// Created by leo on 8/16/25.
//

#include "wrld/logs.hpp"


#include <iostream>
#include <wrld/shaders/vertex/default_shader.hpp>
#include <wrld/shaders/fragment/deferred_pass1_shader.hpp>
#include <wrld/shaders/deferred_pass2_shader.hpp>


#include <wrld/Main.hpp>
#include <wrld/systems/DeferredRendererSystem.hpp>
#include <wrld/systems/RendererSystem.hpp>
#include <wrld/components/Camera3D.hpp>
#include <wrld/components/StaticModel.hpp>

#include <GLFW/glfw3.h>
#include <wrld/tools/Geometry.hpp>

namespace wrld {
    DeferredRendererSystem::DeferredRendererSystem(World &world, GLFWwindow *window) :
        RendererSystem(world, window), vao(0) {
        glGenVertexArrays(1, &vao);

        const auto pass1 = world.create_resource<rsc::Program>("pass1_program");
        pass1->shader_source(rsc::ShaderType::Vertex, shader::DEFAULT_VERTEX);
        pass1->shader_source(rsc::ShaderType::Fragment, shader::DEFERRED_PASS1);
        pass1_program = pass1;

        const auto pass2 = world.create_resource<rsc::Program>("pass2_program");
        pass2->shader_source(rsc::ShaderType::Vertex, shader::DEFERRED_PASS2);
        pass2->shader_source(rsc::ShaderType::Fragment, shader::DEFERRED_PASS2);
        pass2_program = pass2;

        pass1_program->reload();
        pass2_program->reload();

        const int width = Main::get_window_viewport()->get_width();
        const int height = Main::get_window_viewport()->get_height();
        previous_width = width;
        previous_heigth = height;
        framebuffer =
                world.create_resource<rsc::DeferredFramebuffer>("render_framebuffer");
        framebuffer->set_size(width, height);
        framebuffer->recreate();
    }

    DeferredRendererSystem::~DeferredRendererSystem() { glDeleteVertexArrays(1, &vao); }

    void DeferredRendererSystem::render_camera(const cpt::Camera3D &camera) {
        //  FIRST PASS

        // Resize buffer is size is updated
        const int width = Main::get_window_viewport()->get_width();
        const int height = Main::get_window_viewport()->get_height();

        if (width != previous_width || height != previous_heigth) {
            previous_width = width;
            previous_heigth = height;
            framebuffer->set_size(width, height);
            framebuffer->recreate();
        }

        const glm::mat4x4 view_matrix = camera.get_view_matrix();
        const glm::mat4x4 projection_matrix = camera.get_projection_matrix();

        // Can't use AABoundingBox because it has a vtable pointer
        // (40 bytes instead of 32 bytes)
        struct AABB {
            glm::vec4 lower;
            glm::vec4 upper;
        };

        // Use compute shader to check visibility
        frustum_culling_program->use();
        frustum_culling_program->set_uniform("view_matrix", camera.get_view_matrix());
        frustum_culling_program->set_uniform("proj_matrix", camera.get_projection_matrix());

        // Collect transform matrix & bounding box of objects
        std::vector model_entities =
                world.get_entities_with_component<cpt::StaticModel>();

        int count = model_entities.size();

        std::vector<glm::mat4> model_matrices;
        model_matrices.reserve(count);
        std::vector<AABB> bounding_boxes;
        bounding_boxes.reserve(count);

        for (const auto entity: model_entities) {
            const auto trsfrm_opt = world.get_component_opt<cpt::Transform>(entity);
            const auto matrix = trsfrm_opt.has_value() ? trsfrm_opt.value()->model_matrix() : glm::mat4{};
            const auto aabb = world.get_component<cpt::StaticModel>(entity)->get_model()->get_bounding_box();
            model_matrices.push_back(matrix);
            bounding_boxes.emplace_back(glm::vec4{aabb.get_lower(), 1.0}, glm::vec4{aabb.get_upper(), 1.0});
        }

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, model_matrices_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::mat4x4) * count, model_matrices.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, aabb_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(AABB) * count, bounding_boxes.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, result_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER,  sizeof(GLint) * count, nullptr, GL_DYNAMIC_DRAW);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, model_matrices_buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, aabb_buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, result_buffer);

        // Start & wait the results
        glDispatchCompute(1, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        // Query the result
        std::vector<GLint> results(count);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, result_buffer);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLint) * count, results.data());

        framebuffer->use();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        pass1_program->use();
        pass1_program->set_uniform("view", view_matrix);
        pass1_program->set_uniform("projection", projection_matrix);

        // Find each entity with a model, get its transform, and render it.
        unsigned visible_models = 0;
        const bool do_culling = camera.is_culling();
        model_entities =
                world.get_entities_with_component<cpt::StaticModel>();
        for (const auto [i, entity]: model_entities | std::views::enumerate) {
            if (do_culling && !results[i]) continue;

            visible_models += 1;

            const auto model_cmpnt =
                    world.get_component_opt<cpt::StaticModel>(entity).value();
            const auto &model = model_cmpnt->get_model();

            glm::mat4x4 model_matrix = get_entity_transform(entity);

            // Actual draw call
            draw_model(model.get_ref(), model_matrix, pass1_program.get_ref());
        }

        unsigned total_models = model_entities.size();
        Main::set_statistic("Visible model count (deferred)",
                            std::format("{}/{}", visible_models, total_models));

        // SECOND PASS
        const auto &window_fb = Main::get_window_viewport();

        // Copy depth buffer from DeferredFramebuffer to window framebuffer
        glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer.get()->get_fbo());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, window_fb->get_fbo());
        glBlitFramebuffer(0, 0, framebuffer.get()->get_width(),
                          framebuffer.get()->get_height(), 0, 0, window_fb->get_width(),
                          window_fb->get_height(), GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glClear(GL_COLOR_BUFFER_BIT);

        window_fb->use();

        pass2_program->use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, framebuffer->get_position_texture());
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, framebuffer->get_normal_texture());
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, framebuffer->get_diffuse_texture());
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, framebuffer->get_do_lighting_texture());

        pass2_program->set_uniform("position_texture", 0);
        pass2_program->set_uniform("normal_texture", 1);
        pass2_program->set_uniform("diffuse_texture", 2);
        pass2_program->set_uniform("do_lighting_texture", 3);

        pass2_program->set_uniform("view_pos", camera.get_position());

        // Light information
        const std::vector<PointLightData> point_lights = get_point_lights();
        const std::vector<DirectionalLightData> directional_lights =
                get_directional_lights();

        const EnvironmentData environment_data = get_environment(camera);

        // Ambiant light uniform
        pass2_program->set_uniform("ambiant_light.color",
                                   environment_data.ambiant_light.color);
        pass2_program->set_uniform("ambiant_light.intensity",
                                   environment_data.ambiant_light.intensity);

        // Point light dependent uniforms
        pass2_program->set_uniform("point_light_nb",
                                   static_cast<unsigned>(point_lights.size()));
        for (const auto &[i, pl]: std::views::enumerate(point_lights)) {
            pass2_program->set_uniform(std::format("point_lights[{}].position", i),
                                       pl.position);
            pass2_program->set_uniform(std::format("point_lights[{}].color", i),
                                       pl.color);
            pass2_program->set_uniform(std::format("point_lights[{}].intensity", i),
                                       pl.intensity);
        }

        // Directional light dependent uniforms
        pass2_program->set_uniform("directional_lights_nb",
                                   static_cast<unsigned>(directional_lights.size()));
        for (const auto &[i, dl]: std::views::enumerate(directional_lights)) {
            pass2_program->set_uniform(std::format("directional_lights[{}].direction", i),
                                       dl.direction);
            pass2_program->set_uniform(std::format("directional_lights[{}].color", i),
                                       dl.color);
            pass2_program->set_uniform(std::format("directional_lights[{}].intensity", i),
                                       dl.intensity);
        }

        glDisable(GL_DEPTH_TEST);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        glEnable(GL_DEPTH_TEST);

        if (environment_data.skybox.has_value()) {
            draw_skybox(environment_data.skybox.value().get_ref(), camera,
                        environment_data.vao);
        }
    }
} // namespace wrld
