//
// Created by leo on 12/16/25.
//

#include <wrld/systems/TAARenderSystem.hpp>
#include <wrld/Main.hpp>
#include <wrld/components/DirectionalLight.hpp>
#include <wrld/components/PointLight.hpp>
#include <wrld/shaders/deferred_second_pass.glsl.hpp>

#include "wrld/logs.hpp"

namespace wrld::sys {
    TAARenderSystem::TAARenderSystem() : DeferredRenderSystem() /*fbo(0),*/ /*history_texture(0), depth_texture(0) */ {
    }

    void TAARenderSystem::init(World &world) {
        DeferredRenderSystem::init(world);

        const int width = Main::get_window_viewport()->get_width();
        const int height = Main::get_window_viewport()->get_height();


        output_framebuffer = world.create_resource<rsc::Framebuffer>("Output Framebuffer");
        output_framebuffer->set_size(width, height);
        output_framebuffer->recreate();
    }

    void TAARenderSystem::render(World &world) {
        const LightCollection light_collection = sample_lights(world);

        if (const auto cameras = world.get_entities_with_component<cpt::Camera3D>();
            !cameras.empty()) {
            const auto camera = world.get_component<cpt::Camera3D>(cameras[0]);
            render_camera(world, *camera, light_collection);
        }

        current_sample_pass = (current_sample_pass + 1) % TAA_IMAGE_COUNT;
    }

    RenderSystem::LightCollection TAARenderSystem::sample_lights(World &world) const {
        return LightCollection{sample_directional_lights(world), sample_point_lights(world)};
    }

    float TAARenderSystem::get_alpha() const {
        return alpha;
    }

    void TAARenderSystem::set_alpha(const float alpha) {
        if (alpha < 0 || alpha > 1) {
            throw std::runtime_error("Alpha should be between 0 and 1");
        }
        this->alpha = alpha;
    }

    void TAARenderSystem::render_camera(World &world, const cpt::Camera3D &camera, const LightCollection &lights) {
        render_camera_first_pass(world, camera);
        render_camera_second_pass(world, camera, lights);
    }

    std::vector<RenderSystem::PointLightData> TAARenderSystem::sample_point_lights(World &world) {
        std::vector<PointLightData> res;

        // Query each PointLight components in world
        auto entities = world.get_entities_with_component<cpt::PointLight>();
        if (entities.size() > MAX_LIGHTS)
            entities.resize(MAX_LIGHTS);

        // Sample
        const size_t count = entities.size() / TAA_IMAGE_COUNT;
        const size_t offset = count * TAA_IMAGE_COUNT;
        std::vector<EntityID> sample;
        sample.reserve(count);
        for (auto it = entities.begin() + offset; it != entities.end(); ++it) {
            sample.push_back(*it);
            if (sample.size() == count) break;
        }


        for (const auto &entity: sample) {
            const auto cpnt = world.get_component<cpt::PointLight>(entity);
            glm::vec3 position = world.get_component<cpt::Transform>(entity)->get_position();
            res.emplace_back(position, cpnt->get_color(), cpnt->get_intensity());
        }

        return res;
    }

    std::vector<RenderSystem::DirectionalLightData> TAARenderSystem::sample_directional_lights(World &world) const {
        std::vector<DirectionalLightData> res;

        // Query each DirectionalLight components in world
        auto entities = world.get_entities_with_component<cpt::DirectionalLight>();
        if (entities.size() > MAX_LIGHTS)
            entities.resize(MAX_LIGHTS);

        // Sample
        const size_t count = entities.size() / TAA_IMAGE_COUNT;
        const size_t offset = count * current_sample_pass;
        std::vector<EntityID> sample;
        sample.reserve(count);
        for (auto it = entities.begin() + offset; it != entities.end(); ++it) {
            sample.push_back(*it);
            if (sample.size() == count) break;
        }

        for (const auto &entity: sample) {
            const auto cpnt = world.get_component<cpt::DirectionalLight>(entity);
            glm::vec3 direction = world.get_component<cpt::Transform>(entity)->forward();
            res.emplace_back(direction, cpnt->get_color(), cpnt->get_intensity());
        }

        return res;
    }

    void TAARenderSystem::create_second_pass(World &world) {
        deferred_second_pass = world.create_resource<rsc::Program>("TAA Second Deferred Pass Shader");
        deferred_second_pass->shader_path(rsc::ShaderType::Vertex, "data/compute/taa_deferred_second_pass.glsl");
        deferred_second_pass->shader_path(rsc::ShaderType::Fragment, "data/compute/taa_deferred_second_pass.glsl");
        deferred_second_pass->reload();
    }

    void TAARenderSystem::create_framebuffer(World &world) {
        const int width = Main::get_window_viewport()->get_width();
        const int height = Main::get_window_viewport()->get_height();

        g_framebuffer = world.create_resource<rsc::TAAFramebuffer>("TAA Framebuffer");
        g_framebuffer->set_size(width, height);
        g_framebuffer->recreate();
    }

    void TAARenderSystem::update_framebuffer() {
        const int width = Main::get_window_viewport()->get_width();
        const int height = Main::get_window_viewport()->get_height();

        if (width != previous_width || height != previous_height) {
            previous_width = width;
            previous_height = height;
            g_framebuffer->set_size(width, height);
            g_framebuffer->recreate();

            output_framebuffer->set_size(width, height);
            output_framebuffer->recreate();
        }
    }

    void TAARenderSystem::render_camera_second_pass(World &world, const cpt::Camera3D &camera,
                                                    const LightCollection &lights) const {
        const auto [ambiant_light, skybox, vao] = get_environment(world, camera);

        // Copy depth buffer from DeferredFramebuffer to window framebuffer
        glBindFramebuffer(GL_READ_FRAMEBUFFER, g_framebuffer->get_fbo());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, Main::get_window_viewport()->get_fbo());
        glBlitFramebuffer(0, 0, previous_width, previous_height,
                          0, 0, previous_width, previous_height,
                          GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glClear(GL_COLOR_BUFFER_BIT);

        output_framebuffer->use();
        deferred_second_pass->use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, g_framebuffer->get_position_texture());
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, g_framebuffer->get_normal_texture());
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, g_framebuffer->get_diffuse_texture());
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, g_framebuffer->get_do_lighting_texture());
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, g_framebuffer->get_history_texture());

        deferred_second_pass->set_uniform("position_texture", 0);
        deferred_second_pass->set_uniform("normal_texture", 1);
        deferred_second_pass->set_uniform("diffuse_texture", 2);
        deferred_second_pass->set_uniform("do_lighting_texture", 3);

        // TAA specific
        deferred_second_pass->set_uniform("history_texture", 4);
        deferred_second_pass->set_uniform("alpha", alpha);

        deferred_second_pass->set_uniform("view_pos", camera.get_position());
        set_scene_uniforms(deferred_second_pass, ambiant_light, lights);

        glDisable(GL_DEPTH_TEST);
        glBindVertexArray(deferred_vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);
        glEnable(GL_DEPTH_TEST);

        // At this point the output image is in output_framebuffer
        // We have to copy it twice : first onto the back buffer of the window
        // second in the history buffer

        glBindFramebuffer(GL_READ_FRAMEBUFFER, output_framebuffer->get_fbo());
        glReadBuffer(GL_COLOR_ATTACHMENT0);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, Main::get_window_viewport()->get_fbo());
        constexpr GLenum buffers[] = {GL_BACK_LEFT};
        glDrawBuffers(1, buffers);

        const auto width = Main::get_window_viewport()->get_width();
        const auto height = Main::get_window_viewport()->get_height();

        // Copy from output_framebuffer to window framebuffer
        glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
                          GL_NEAREST);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, g_framebuffer->get_fbo());
        constexpr GLenum buffers2[] = {GL_COLOR_ATTACHMENT4};
        glDrawBuffers(1, buffers2);

        // Copy from output_framebuffer to g_framebuffer history texture
        glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

        Main::get_window_viewport()->use();

        if (skybox.has_value()) {
            draw_skybox(skybox.value().get_ref(), camera, vao);
        }
    }

    Rc<rsc::DeferredFramebuffer> TAARenderSystem::get_framebuffer() const {
        return g_framebuffer.as<rsc::DeferredFramebuffer>();
    }
}
