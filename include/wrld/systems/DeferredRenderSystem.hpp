//
// Created by leo on 12/8/25.
//

#pragma once
#include <wrld/systems/ForwardRenderSystem.hpp>
#include <wrld/resources/DeferredFramebuffer.hpp>

namespace wrld::sys {
    class DeferredRenderSystem : public RenderSystem,
                                 public Singleton<DeferredRenderSystem> {
    public:
        /// Initialize the system, compiles required shaders...
        void init(World &world) override;

        /// Render all the cameras in the world.
        void render(World &world) override;

    protected:
        // ------------------ Private functions ------------------ //

        /// Render the given camera.
        void render_camera(World &world, const cpt::Camera3D &camera,
                           const LightCollection &lights);

        /// First pass of the deferred rendering of the given camera.
        void render_camera_first_pass(World &world, const cpt::Camera3D &camera);

        /// Second pass of the deferred rendering of the given camera.
        /// DeferredRenderSystem::render_camera_first_pass should be called first.
        void render_camera_second_pass(World &world, const cpt::Camera3D &camera, const LightCollection &lights) const;

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
