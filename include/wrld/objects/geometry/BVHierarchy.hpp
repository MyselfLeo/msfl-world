//
// Created by leo on 11/19/25.
//

#pragma once
#include <wrld/objects/geometry/AABoundingBox.hpp>

namespace wrld::obj {
    struct BVHNode {
        glm::vec3 lower;
        glm::vec3 upper;

        BVHNode *left_child;
        BVHNode *right_child;
    };

    /// Binary tree for fast geometrical requests.
    /// Must be built in the use space, as it can't really be
    /// transformed from one space to another.
    class BVHierarchy {};

} // namespace wrld::obj
