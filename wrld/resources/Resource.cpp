//
// Created by leo on 9/6/25.
//

#include "Resource.hpp"

#include <utility>

namespace wrld {
    Resource::Resource(std::string name, World &world) : name(std::move(name)), world(world) {}

    std::string Resource::get_name() const { return name; }

    void Resource::attach_component_user(const std::shared_ptr<const Component> &user) {
        const Component *ptr = user.get();

        if (!component_users.contains(std::type_index(typeid(*ptr)))) {
            component_users[std::type_index(typeid(*ptr))] = {};
        }
        component_users[std::type_index(typeid(*ptr))][ptr] = user;
    }

    void Resource::detach_component_user(const Component *user) {
        component_users[std::type_index(typeid(user))].erase(user);
    }

    void Resource::attach_resource_user(const std::shared_ptr<const Resource> &user) {
        const Resource *ptr = user.get();

        if (!resource_users.contains(std::type_index(typeid(*ptr)))) {
            resource_users[std::type_index(typeid(*ptr))] = {};
        }
        resource_users[std::type_index(typeid(*ptr))][ptr] = user;
    }

    void Resource::detach_resource_user(const Resource *user) {
        resource_users[std::type_index(typeid(user))].erase(user);
    }

    void Resource::attach_resource(const std::string &unique_name, const std::shared_ptr<Resource> &resource) {
        if (attached_resources.contains(unique_name)) {
            attached_resources[unique_name]->detach_resource_user(this);
        }

        resource->attach_resource_user(shared_from_this());
        attached_resources[unique_name] = resource;
    }

} // namespace wrld
