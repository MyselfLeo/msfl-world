//
// Created by leo on 9/6/25.
//

#include "Resource.hpp"

namespace wrld::rsc {
    Resource::Resource(const ResourceID resource_id) : resource_id(resource_id) {}

    ResourceID Resource::get_id() const { return resource_id; }
} // namespace wrld::rsc
