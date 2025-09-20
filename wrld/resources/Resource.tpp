//
// Created by leo on 9/20/25.
//

#pragma once
#include "Rc.hpp"

namespace wrld {

    template<ResourceConcept R>
    void Resource::attach_resource(const std::string &unique_name, const Rc<R> &resource) {
        if (attached_resources.contains(unique_name)) {
            attached_resources[unique_name].detach_resource_user<R>(get_name());
        }

        resource.attach_resource_user(std::type_index(typeid(*this)), get_name());
        attached_resources[unique_name] = resource.template as<Resource>();
    }

    template<ResourceConcept R>
    Rc<R> Resource::get_resource(const std::string &unique_name) const {
        if (!attached_resources.contains(unique_name)) {
            throw std::runtime_error(std::format("Tried to access unbound resource {}", unique_name));
        }
        return attached_resources.at(unique_name).as<R>();
    }
} // namespace wrld
