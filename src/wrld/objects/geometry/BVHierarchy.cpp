//
// Created by leo on 11/19/25.
//

#include <algorithm>
#include <stdexcept>
#include <wrld/objects/geometry/BVHierarchy.hpp>

namespace wrld::obj {
    BVHLeaf::BVHLeaf(const std::span<unsigned int> &element_ids) :
        element_ids(element_ids) {}
} // namespace wrld::obj
