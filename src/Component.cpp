//
// Created by leo on 8/11/25.
//

#include "Component.hpp"

namespace wrld {
    Component::Component(EntityID entity_id, World &world) : entity_id(entity_id), world(world) {}

} // namespace wrld
