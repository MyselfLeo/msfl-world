//
// Created by leo on 8/11/25.
//

#include "Component.hpp"

namespace wrld::cpt {
    Component::Component(const EntityID entity_id, World &world) : entity_id(entity_id), world(world) {}

    EntityID Component::get_entity() const { return entity_id; }

} // namespace wrld::cpt
