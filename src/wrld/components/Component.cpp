//
// Created by leo on 8/11/25.
//

#include <wrld/components/Component.hpp>
#include <wrld/resources/Resource.hpp>
#include <wrld/resources/Rc.hpp>

namespace wrld {
    Component::Component(const EntityID entity_id, World &world) : entity_id(entity_id), world(world) {}

    EntityID Component::get_entity() const { return entity_id; }

    bool Component::has_resource(const std::string &unique_name) const {
        return attached_resources.contains(unique_name);
    }

    void Component::detach_resource(const std::string &unique_name) {
        if (!attached_resources.contains(unique_name))
            return;

        attached_resources[unique_name].detach_component_user(std::type_index(typeid(*this)), get_entity());
    }

    // void Component::detach_resource(const std::string &unique_name) {
    //     if (!attached_resources.contains(unique_name))
    //         return;
    //
    //     attached_resources[unique_name].detach_component_user(this);
    // }

} // namespace wrld
