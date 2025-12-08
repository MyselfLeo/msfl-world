//
// Created by leo on 12/8/25.
//

#pragma once
#include <wrld/systems/ForwardRenderSystem.hpp>

namespace wrld::sys {
    class DeferredRenderSystem final : public ForwardRenderSystem {
    public:
        DeferredRenderSystem();

        /// Initialize the system, compiles required shaders...
        void init(World &world) override;

    private:
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
        int previous_heigth = 0;
    };
}
