//
// Created by leo on 8/16/25.
//

#pragma once

#include <wrld/components/Camera3D.hpp>
#include <wrld/systems/RendererSystem.hpp>
#include <wrld/resources/DeferredFramebuffer.hpp>

namespace wrld {
    class DeferredRendererSystem : public RendererSystem {
    public:
        // static constexpr unsigned MAX_LIGHTS = 100;

        DeferredRendererSystem(World &world, GLFWwindow *window);


        ~DeferredRendererSystem() override;

        // void exec() override;
        // [[nodiscard]] GLFWwindow *get_window() const;

    private:
        GLuint vao;

        Rc<rsc::Program> pass1_program;
        Rc<rsc::Program> pass2_program;

        Rc<rsc::DeferredFramebuffer> framebuffer;

        void render_camera(const cpt::Camera3D &camera) override;
    };
} // namespace wrld
