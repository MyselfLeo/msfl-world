#pragma once
#include "Component.hpp"
#include "resources/Rc.hpp"

namespace wrld {

    template<ResourceConcept R>
    void Component::attach_resource(const std::string &unique_name, const Rc<R> &resource) {
        if (attached_resources.contains(unique_name)) {
            attached_resources[unique_name].detach_component_user(std::type_index(typeid(*this)), get_entity());
        }
        resource.attach_component_user(std::type_index(typeid(*this)), get_entity());
        attached_resources[unique_name] = resource.template as<Resource>();
    }

    template<ResourceConcept R>
    Rc<R> Component::get_resource(const std::string &unique_name) const {
        if (!attached_resources.contains(unique_name)) {
            throw std::runtime_error(std::format("Tried to access unbound resource {}", unique_name));
        }
        return attached_resources.at(unique_name).as<R>();
    }
} // namespace wrld
