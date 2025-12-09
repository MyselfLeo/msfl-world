//
// Created by leo on 11/28/25.
//

#include <wrld/systems/ForwardRenderSystem.hpp>

#include "wrld/Main.hpp"
#include "wrld/World.hpp"
#include "wrld/components/DirectionalLight.hpp"
#include "wrld/components/PointLight.hpp"
#include "wrld/components/StaticModel.hpp"
#include "wrld/shaders/deferred_second_pass.glsl.hpp"
#include "wrld/shaders/skybox_shader.glsl.hpp"
#include "wrld/shaders/compute/visibility_check.comp.hpp"
#include "wrld/shaders/compute/draw_call_gen.comp.hpp"
#include "wrld/shaders/fragment/deferred.frag.hpp"
#include "wrld/shaders/fragment/forward.frag.hpp"
#include "wrld/shaders/vertex/deferred.vert.hpp"
#include "wrld/shaders/vertex/forward.vert.hpp"

namespace wrld::sys {
    void ForwardRenderSystem::init(World &world) {
        RenderSystem::init(world);

        forward_program = world.create_resource<rsc::Program>("Forward Shader");
        forward_program->shader_source(rsc::ShaderType::Vertex, shader::vert::FORWARD);
        forward_program->shader_source(rsc::ShaderType::Fragment, shader::frag::FORWARD);
        forward_program->reload();
    }

    void ForwardRenderSystem::render(World &world) {
        const LightCollection light_collection{
            get_directional_lights(world),
            get_point_lights(world),
        };

        if (const auto cameras = world.get_entities_with_component<cpt::Camera3D>();
            !cameras.empty()) {
            const auto camera = world.get_component<cpt::Camera3D>(cameras[0]);
            render_camera(world, *camera, light_collection);
        }
    }

    void ForwardRenderSystem::render_camera(World &world, const cpt::Camera3D &camera, const LightCollection &lights) {
        // todo: In the future, a camera should always be attached to a framebuffer, and get rendered
        // on this framebuffer. (a "Viewport" ?)
        // Draw skybox if there is one
        const auto [ambiant_light, skybox, vao] = get_environment(world, camera);
        if (skybox.has_value()) {
            draw_skybox(skybox.value().get_ref(), camera,
                        vao);
        }

        // Compute visibility of each object
        compute_renderables_visiblity(world, camera);

        set_scene_uniforms(forward_program, ambiant_light, lights);
        set_camera_uniforms(forward_program, camera);

        glBindBuffer(GL_PARAMETER_BUFFER_ARB, arb_counter_buffer);
        glBindVertexArray(static_models_vao);

        Main::get_window_viewport()->use();

        // For each material, we fill the ARB buffer and we do a render pass
        for (const auto &[mat_idx, mat]: materials | std::views::enumerate) {
            compute_draw_calls_for_material(mat_idx);

            forward_program->use();
            forward_program->set_uniform("material", mat);

            glDepthMask(mat->is_doing_depth_mask());

            // Actual draw calls (point, line, triangle)
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            forward_program->set_uniform("polygon_mode", 0);

            glBindBuffer(GL_DRAW_INDIRECT_BUFFER, triangles_indirect_draw_buffer);
            bind_trsfm_buffer(obj::PrimitiveType::Triangles);
            glMultiDrawElementsIndirectCountARB(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, 2 * sizeof(GLsizei),
                                                std::get<2>(max_mesh_count), 0);

            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            forward_program->set_uniform("polygon_mode", 1);
            glLineWidth(mat->get_line_width());

            glBindBuffer(GL_DRAW_INDIRECT_BUFFER, lines_indirect_draw_buffer);
            bind_trsfm_buffer(obj::PrimitiveType::Lines);
            glMultiDrawElementsIndirectCountARB(GL_LINES, GL_UNSIGNED_INT, nullptr, 1 * sizeof(GLsizei),
                                                std::get<1>(max_mesh_count), 0);

            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            forward_program->set_uniform("polygon_mode", 2);
            glPointSize(mat->get_point_size());

            glBindBuffer(GL_DRAW_INDIRECT_BUFFER, points_indirect_draw_buffer);
            bind_trsfm_buffer(obj::PrimitiveType::Points);
            glMultiDrawElementsIndirectCountARB(GL_POINTS, GL_UNSIGNED_INT, nullptr, 0 * sizeof(GLsizei),
                                                std::get<0>(max_mesh_count), 0);
        }
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);
        glBindVertexArray(0);
    }
} // wrld
