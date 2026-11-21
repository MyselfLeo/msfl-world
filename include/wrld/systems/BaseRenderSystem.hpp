//
// Created by myselfleo on 21/11/2026.
//

#pragma once
#include "wrld/World.hpp"

namespace wrld::sys {
    class BaseRenderSystem {
    public:
        virtual ~BaseRenderSystem() = default;

        /// Initialize the system, compiles required shaders...
        virtual void init(World &world) = 0;

        /// Render all the cameras in the world.
        virtual void render(World &world) = 0;

        /// Reload loaded meshes, models & materials. All the data
        /// is sent to the GPU.
        virtual void reload_resources(const World &world) = 0;
    };
}
