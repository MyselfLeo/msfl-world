//
// Created by leo on 9/6/25.
//

#include "Resource.hpp"

#include <utility>

namespace wrld::rsc {
    Resource::Resource(std::string name, World &world) :
        name(std::move(name)), world(world) {}

    // ResourceID Resource::get_id() const { return resource_id; }

    std::string Resource::get_name() const { return name; }
} // namespace wrld::rsc
