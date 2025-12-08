//
// Created by leo on 8/16/25.
//

#pragma once

#include <wrld/components/Camera3D.hpp>
#include <wrld/systems/OLDRendererSystem.hpp>
#include <wrld/resources/DeferredFramebuffer.hpp>

namespace wrld {
    class OLDDeferredRendererSystem : public OLDRendererSystem {
    public:
        // static constexpr unsigned MAX_LIGHTS = 100;

        OLDDeferredRendererSystem(World &world, GLFWwindow *window);


        ~OLDDeferredRendererSystem() override;

        // void exec() override;
        // [[nodiscard]] GLFWwindow *get_window() const;

    private:
        int previous_width;
        int previous_heigth;

        GLuint vao;

        Rc<rsc::Program> pass1_program;
        Rc<rsc::Program> pass2_program;

        Rc<rsc::DeferredFramebuffer> framebuffer;

        void render_camera(const cpt::Camera3D &camera) override;
    };
} // namespace wrld
