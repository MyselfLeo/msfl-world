//
// Created by leo on 8/11/25.
//

#include "Component.hpp"

#include "resources/Resource.hpp"

namespace wrld {
    Component::Component(const EntityID entity_id, World &world) : entity_id(entity_id), world(world) {}

    EntityID Component::get_entity() const { return entity_id; }

    void Component::attach_resource(const std::string &unique_name, const std::shared_ptr<Resource> &resource) {
        if (attached_resources.contains(unique_name)) {
            attached_resources[unique_name]->detach_component_user(this);
        }

        resource->attach_component_user(shared_from_this());
        attached_resources[unique_name] = resource;
    }

    void Component::detach_resource(const std::string &unique_name) {
        if (!attached_resources.contains(unique_name))
            return;

        attached_resources[unique_name]->detach_component_user(this);
    }

} // namespace wrld
