//
// Created by leo on 12/8/25.
//

#pragma once
#include <wrld/systems/ForwardRenderSystem.hpp>

namespace wrld::sys {
    class DeferredRenderSystem final : public RenderSystem, public Singleton<DeferredRenderSystem> {
    public:
        /// Initialize the system, compiles required shaders...
        void init(World &world) override;

        /// Render all the cameras in the world.
        void render(World &world) override;

    private:
        // ------------------ Private functions ------------------ //

        /// Render the given camera.
        void render_camera(World &world, const cpt::Camera3D &camera,
                           const LightCollection &lights);

        /// Update the framebuffer if the window size has changed.
        /// todo: When viewports are bound to a camera, we check the size
        /// of this viewport instead.
        void update_framebuffer();

        // ------------------ Resources ------------------ //

        /// First step of the deferred rendering process (Vertex + Fragment).
        /// This program will write relevant data into a DeferredFramebuffer.
        Rc<rsc::Program> deferred_first_pass;

        /// Second step of the deferred rendering process (Vertex + Fragment).
        /// This program takes the data from the previous Framebuffer and renders
        /// it correctly with all the materials applied.
        Rc<rsc::Program> deferred_second_pass;

        /// Framebuffer used for deferred rendering
        Rc<rsc::DeferredFramebuffer> framebuffer;

        /// Track the size of the viewport to update the framebuffer
        /// when required
        int previous_width = 0;
        int previous_height = 0;

        /// VAO used for the 2nd pass of the deferred rendering
        GLuint deferred_vao = 0;
    };
}
