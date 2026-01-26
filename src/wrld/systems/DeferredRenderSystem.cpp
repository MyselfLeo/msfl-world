//
// Created by leo on 12/8/25.
//

#include <wrld/systems/DeferredRenderSystem.hpp>

#include <wrld/Main.hpp>
#include <wrld/shaders/deferred_second_pass.glsl.hpp>
#include <wrld/shaders/fragment/deferred.frag.hpp>
#include <wrld/shaders/vertex/deferred.vert.hpp>

namespace wrld::sys {
    void DeferredRenderSystem::init(World &world) {
        RenderSystem::init(world);
        create_first_pass(world);
        create_second_pass(world);
        const int width = Main::get_window_viewport()->get_width();
        const int height = Main::get_window_viewport()->get_height();
        previous_width = width;
        previous_height = height;

        create_framebuffer(world);

        glGenVertexArrays(1, &deferred_vao);
    }

    void DeferredRenderSystem::render(World &world) {
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

    void DeferredRenderSystem::render_camera(World &world, const cpt::Camera3D &camera, const LightCollection &lights) {
        render_camera_first_pass(world, camera);
        render_camera_second_pass(world, camera, lights);
    }

    void DeferredRenderSystem::render_camera_first_pass(World &world, const cpt::Camera3D &camera) {
        update_framebuffer();

        get_framebuffer()->use();
        glClearColor(0, 0, 0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Compute visibility of each object
        compute_renderables_visiblity(world, camera);
        set_program_uniforms(deferred_first_pass);
        set_camera_uniforms(deferred_first_pass, camera);

        glBindBuffer(GL_PARAMETER_BUFFER_ARB, arb_counter_buffer);

        // First pass : render to the DeferredFramebuffer
        // For each material, we fill the ARB buffer and we do a render pass
        for (const auto &[mat_idx, mat]: materials | std::views::enumerate) {
            Main::get_window_viewport()->use();
            compute_draw_calls_for_material(mat_idx);

            get_framebuffer()->use();

            deferred_first_pass->use();
            deferred_first_pass->set_uniform("material", mat);

            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);
            glDepthMask(mat->is_doing_depth_mask());

            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            deferred_first_pass->set_uniform("polygon_mode", 0);

            glBindBuffer(GL_DRAW_INDIRECT_BUFFER, triangles_indirect_draw_buffer);
            glBindVertexArray(static_models_vao);
            bind_trsfm_buffer(obj::PrimitiveType::Triangles);
            glMultiDrawElementsIndirectCountARB(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, 2 * sizeof(GLsizei),
                                                max_mesh_count, 0);

            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            deferred_first_pass->set_uniform("polygon_mode", 1);
            glLineWidth(mat->get_line_width());

            glBindBuffer(GL_DRAW_INDIRECT_BUFFER, lines_indirect_draw_buffer);
            bind_trsfm_buffer(obj::PrimitiveType::Lines);
            glMultiDrawElementsIndirectCountARB(GL_LINES, GL_UNSIGNED_INT, nullptr, 1 * sizeof(GLsizei),
                                                max_mesh_count, 0);

            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            deferred_first_pass->set_uniform("polygon_mode", 2);
            glPointSize(mat->get_point_size());

            glBindBuffer(GL_DRAW_INDIRECT_BUFFER, points_indirect_draw_buffer);
            bind_trsfm_buffer(obj::PrimitiveType::Points);
            glMultiDrawElementsIndirectCountARB(GL_POINTS, GL_UNSIGNED_INT, nullptr, 0 * sizeof(GLsizei),
                                                max_mesh_count, 0);
        }

        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);
        glBindVertexArray(0);

        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    void DeferredRenderSystem::render_camera_second_pass(World &world, const cpt::Camera3D &camera,
                                                         const LightCollection &lights) {
        const auto [ambiant_light, skybox, vao] = get_environment(world, camera);

        // Copy depth buffer from DeferredFramebuffer to window framebuffer
        glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer->get_fbo());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, Main::get_window_viewport()->get_fbo());
        glBlitFramebuffer(0, 0, previous_width, previous_height,
                          0, 0, previous_width, previous_height,
                          GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glClear(GL_COLOR_BUFFER_BIT);

        Main::get_window_viewport()->use();
        deferred_second_pass->use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, framebuffer->get_position_texture());
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, framebuffer->get_normal_texture());
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, framebuffer->get_diffuse_texture());
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, framebuffer->get_do_lighting_texture());

        deferred_second_pass->set_uniform("position_texture", 0);
        deferred_second_pass->set_uniform("normal_texture", 1);
        deferred_second_pass->set_uniform("diffuse_texture", 2);
        deferred_second_pass->set_uniform("do_lighting_texture", 3);

        deferred_second_pass->set_uniform("view_pos", camera.get_position());
        set_scene_uniforms(deferred_second_pass, ambiant_light, lights);
        set_program_uniforms(deferred_second_pass);

        glDisable(GL_DEPTH_TEST);
        glBindVertexArray(deferred_vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);
        glEnable(GL_DEPTH_TEST);

        if (skybox.has_value()) {
            draw_skybox(skybox.value().get_ref(), camera, vao);
        }
    }

    void DeferredRenderSystem::create_first_pass(World &world) {
        deferred_first_pass = world.create_resource<rsc::Program>("First Deferred Pass Shader");
        deferred_first_pass->shader_source(rsc::ShaderType::Vertex, shader::vert::DEFERRED_FIRST_PASS);
        deferred_first_pass->shader_source(rsc::ShaderType::Fragment, shader::frag::DEFERRED_FIRST_PASS);
        deferred_first_pass->reload();
    }

    void DeferredRenderSystem::create_second_pass(World &world) {
        deferred_second_pass = world.create_resource<rsc::Program>("Second Deferred Pass Shader");
        deferred_second_pass->shader_source(rsc::ShaderType::Vertex, shader::DEFERRED_SECOND_PASS);
        deferred_second_pass->shader_source(rsc::ShaderType::Fragment, shader::DEFERRED_SECOND_PASS);
        deferred_second_pass->reload();
    }

    void DeferredRenderSystem::create_framebuffer(World &world) {
        const int width = Main::get_window_viewport()->get_width();
        const int height = Main::get_window_viewport()->get_height();

        framebuffer = world.create_resource<rsc::DeferredFramebuffer>("Deferred Framebuffer");
        framebuffer->set_size(width, height);
        framebuffer->recreate();
    }

    void DeferredRenderSystem::update_framebuffer() {
        const int width = Main::get_window_viewport()->get_width();
        const int height = Main::get_window_viewport()->get_height();

        if (width != previous_width || height != previous_height) {
            previous_width = width;
            previous_height = height;
            framebuffer->set_size(width, height);
            framebuffer->recreate();
        }
    }

    Rc<rsc::DeferredFramebuffer> DeferredRenderSystem::get_framebuffer() const {
        return framebuffer;
    }
}
