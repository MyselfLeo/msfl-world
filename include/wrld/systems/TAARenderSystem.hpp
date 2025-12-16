//
// Created by leo on 12/16/25.
//

#pragma once
#include "DeferredRenderSystem.hpp"

namespace wrld::sys {
    class TAARenderSystem final : public DeferredRenderSystem {
    public:
        TAARenderSystem();

        /// Render all the cameras in the world.
        void render(World &world) override;

    protected:
        GLuint accu_texture = 0;
    };
}
