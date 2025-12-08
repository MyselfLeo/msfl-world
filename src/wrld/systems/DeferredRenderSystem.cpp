//
// Created by leo on 12/8/25.
//

#include <wrld/systems/DeferredRenderSystem.hpp>

#include <wrld/Main.hpp>
#include <wrld/shaders/deferred_second_pass.glsl.hpp>
#include <wrld/shaders/fragment/deferred.frag.hpp>
#include <wrld/shaders/vertex/deferred.vert.hpp>

namespace wrld::sys {
    DeferredRenderSystem::DeferredRenderSystem() : ForwardRenderSystem() {
    }

    void DeferredRenderSystem::init(World &world) {
        ForwardRenderSystem::init(world);

        deferred_first_pass = world.create_resource<rsc::Program>("First Deferred Pass Shader");
        deferred_first_pass->shader_source(rsc::ShaderType::Vertex, shader::vert::DEFERRED_FIRST_PASS);
        deferred_first_pass->shader_source(rsc::ShaderType::Fragment, shader::frag::DEFERRED_FIRST_PASS);
        deferred_first_pass->reload();

        deferred_second_pass = world.create_resource<rsc::Program>("Second Deferred Pass Shader");
        deferred_second_pass->shader_source(rsc::ShaderType::Vertex, shader::DEFERRED_SECOND_PASS);
        deferred_second_pass->shader_source(rsc::ShaderType::Fragment, shader::DEFERRED_SECOND_PASS);
        deferred_second_pass->reload();

        const int width = Main::get_window_viewport()->get_width();
        const int height = Main::get_window_viewport()->get_height();
        previous_width = width;
        previous_heigth = height;

        framebuffer =
                world.create_resource<rsc::DeferredFramebuffer>("render_framebuffer");
        framebuffer->set_size(width, height);
        framebuffer->recreate();
    }
}
