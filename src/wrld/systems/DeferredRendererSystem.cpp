//
// Created by leo on 8/16/25.
//

#include <wrld/shaders/vertex/default_shader.hpp>
#include <wrld/shaders/fragment/deferred_pass1_shader.hpp>
#include <wrld/shaders/deferred_pass2_shader.hpp>


#include <wrld/Main.hpp>
#include <wrld/systems/DeferredRendererSystem.hpp>
#include <wrld/systems/RendererSystem.hpp>
#include <wrld/components/Camera.hpp>
#include <wrld/components/StaticModel.hpp>

#include <GLFW/glfw3.h>

namespace wrld {
    DeferredRendererSystem::DeferredRendererSystem(World &world, GLFWwindow *window) :
        RendererSystem(world, window), vao(0) {
        glGenVertexArrays(1, &vao);

        const auto pass1 = world.create_resource<rsc::Program>("pass1_program");
        pass1.get_mut()->from_source(shader::DEFAULT_VERTEX, shader::DEFERRED_PASS1);
        pass1_program = pass1;

        const auto pass2 = world.create_resource<rsc::Program>("pass2_program");
        pass2.get_mut()->from_source(shader::DEFERRED_PASS2);
        pass2_program = pass2;

        int w, h;
        glfwGetWindowSize(window, &w, &h);
        const auto fb = world.create_resource<rsc::DeferredFramebuffer>("render_framebuffer");
        fb.get_mut()->set_size(w, h);
        framebuffer = fb;
        framebuffer.get_mut()->recreate();
    }

    DeferredRendererSystem::~DeferredRendererSystem() { glDeleteVertexArrays(1, &vao); }

    void DeferredRendererSystem::render_camera(const cpt::Camera &camera) const {
        // wrldInfo("Rendering");
        //  FIRST PASS

        const glm::mat4x4 view_matrix = camera.get_view_matrix();
        const glm::mat4x4 projection_matrix = camera.get_projection_matrix();

        framebuffer.get_mut()->use();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        pass1_program.get_mut()->use();
        pass1_program.get_mut()->set_uniform("view", view_matrix);
        pass1_program.get_mut()->set_uniform("projection", projection_matrix);

        // Find each entity with a model, get its transform, and render it.
        for (const std::vector model_entities = world.get_entities_with_component<cpt::StaticModel>();
             const auto entity: model_entities) {
            const auto model_cmpnt = world.get_component_opt<cpt::StaticModel>(entity).value();
            const auto &model = model_cmpnt->get_model();

            glm::mat4x4 model_matrix = get_entity_transform(entity);

            // Actual draw call
            draw_model(model.get_ref(), model_matrix, pass1_program.get_ref());
        }


        // SECOND PASS
        const auto &window_fb = Main::get_window_viewport();

        // Copy depth buffer from DeferredFramebuffer to window framebuffer
        glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer.get()->get_fbo());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, window_fb->get_fbo());
        glBlitFramebuffer(0, 0, framebuffer.get()->get_width(), framebuffer.get()->get_height(), 0, 0,
                          window_fb->get_width(), window_fb->get_height(), GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glClear(GL_COLOR_BUFFER_BIT);

        window_fb->use();

        pass2_program.get()->use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, framebuffer.get()->get_position_texture());
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, framebuffer.get()->get_normal_texture());
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, framebuffer.get()->get_diffuse_texture());

        pass2_program.get_mut()->set_uniform("position_texture", 0);
        pass2_program.get_mut()->set_uniform("normal_texture", 1);
        pass2_program.get_mut()->set_uniform("diffuse_texture", 2);

        pass2_program.get_mut()->set_uniform("view_pos", camera.get_position());

        // Light information
        const std::vector<PointLightData> point_lights = get_point_lights();
        const std::vector<DirectionalLightData> directional_lights = get_directional_lights();

        const EnvironmentData environment_data = get_environment(camera);

        // Ambiant light uniform
        pass2_program.get_mut()->set_uniform("ambiant_light.color", environment_data.ambiant_light.color);
        pass2_program.get_mut()->set_uniform("ambiant_light.intensity", environment_data.ambiant_light.intensity);

        // Point light dependent uniforms
        pass2_program.get_mut()->set_uniform("point_light_nb", static_cast<unsigned>(point_lights.size()));
        for (const auto &[i, pl]: std::views::enumerate(point_lights)) {
            pass2_program.get_mut()->set_uniform(std::format("point_lights[{}].position", i), pl.position);
            pass2_program.get_mut()->set_uniform(std::format("point_lights[{}].color", i), pl.color);
            pass2_program.get_mut()->set_uniform(std::format("point_lights[{}].intensity", i), pl.intensity);
        }

        // pass2_program->set_uniform("point_light_nb", 1);
        // pass2_program->set_uniform(std::format("point_lights[0].position"), glm::vec3(0, 0, 0));
        // pass2_program->set_uniform(std::format("point_lights[0].color"), glm::vec3(1, 1, 1));
        // pass2_program->set_uniform(std::format("point_lights[0].intensity"), 10.0f);


        // Directional light dependent uniforms
        pass2_program.get_mut()->set_uniform("directional_lights_nb", static_cast<unsigned>(directional_lights.size()));
        for (const auto &[i, dl]: std::views::enumerate(directional_lights)) {
            pass2_program.get_mut()->set_uniform(std::format("directional_lights[{}].direction", i), dl.direction);
            pass2_program.get_mut()->set_uniform(std::format("directional_lights[{}].color", i), dl.color);
            pass2_program.get_mut()->set_uniform(std::format("directional_lights[{}].intensity", i), dl.intensity);
        }

        glDisable(GL_DEPTH_TEST);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        glEnable(GL_DEPTH_TEST);

        if (environment_data.skybox.has_value()) {
            draw_skybox(environment_data.skybox.value().get_ref(), camera, environment_data.vao);
        }
    }
} // namespace wrld
