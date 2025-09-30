//
// Created by leo on 9/29/25.
//

#pragma once
#include <wrld/World.hpp>
#include <wrld/components/Camera.hpp>
#include <wrld/components/Component.hpp>
#include <wrld/components/StaticModel.hpp>
#include <wrld/components/Transform.hpp>

namespace wrld::tools {

    class Geometry {
    public:
        /// Test if the bounding-box of the entity-attached model (or TODO: modelgroup)
        /// is in the frustum of camera.
        static bool is_visible(World &world, EntityID entity, EntityID camera);
    };

} // namespace wrld::tools
