//
// Created by leo on 11/28/25.
//

#pragma once

#include "glad/glad.h"
#include "glm/mat4x4.hpp"

#include <wrld/resources/Rc.hpp>
#include <wrld/resources/Material.hpp>

#include "RenderSystem.hpp"

#include "wrld/components/Camera3D.hpp"
#include "wrld/components/Environment3D.hpp"
#include "wrld/resources/DeferredFramebuffer.hpp"
#include "wrld/resources/Model.hpp"

namespace wrld::sys {
    /// This system manages :
    /// - Allocation of geometry to the GPU
    /// - Rendering pipeline (GPU culling -> Vertex shader -> Fragment shader)
    class ForwardRenderSystem : public RenderSystem, public Singleton<ForwardRenderSystem> {
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

        // ------------------ Resources ------------------ //

        /// Forward program.
        /// todo: Use the camera's program instead.
        Rc<rsc::Program> forward_program;
    };
} // wrld
