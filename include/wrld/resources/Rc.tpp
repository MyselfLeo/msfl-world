//
// Created by leo on 9/20/25.
//

#pragma once

#include <wrld/components/Component.hpp>
#include <wrld/resources/Resource.hpp>

namespace wrld {

    template<ResourceConcept R>
    Rc<R>::Rc() : component_users({}), resource_users({}) {}

    template<ResourceConcept R>
    Rc<R>::Rc(std::string name, World &world) {
        this->component_users = std::make_shared<UserComponentPool>();
        this->resource_users = std::make_shared<UserResourcePool>();
        resource = std::make_shared<R>(name, world /*, this->as_ptr<Resource>()*/);
        // resource->load_default_resources();
    }

    template<ResourceConcept R>
    Rc<R>::Rc(std::shared_ptr<R> ptr, std::shared_ptr<UserComponentPool> comp_users,
              std::shared_ptr<UserResourcePool> res_users) :
        resource(std::move(ptr)), component_users(std::move(comp_users)), resource_users(std::move(res_users)) {}

    template<ResourceConcept R>
    const R *Rc<R>::get() const {
        return resource.get();
    }

    template<ResourceConcept R>
    R *Rc<R>::get_mut() const {
        return resource.get();
    }

    template<ResourceConcept R>
    std::shared_ptr<R> Rc<R>::get_ptr() const {
        return resource;
    }

    template<ResourceConcept R>
    R &Rc<R>::get_ref() const {
        return *resource;
    }

    template<ResourceConcept R>
    template<ComponentConcept T>
    void Rc<R>::attach_component_user(const EntityID id) const {
        const auto &type_index = std::type_index(typeid(T));
        attach_component_user(type_index, id);
    }

    template<ResourceConcept R>
    void Rc<R>::attach_component_user(const std::type_index &ti, const EntityID id) const {
        if (!component_users->contains(ti)) {
            component_users->insert_or_assign(ti, std::unordered_set<EntityID>());
        }
        component_users->at(ti).insert(id);
    }

    template<ResourceConcept R>
    template<ComponentConcept T>
    void Rc<R>::detach_component_user(const EntityID id) const {
        const auto &type_index = std::type_index(typeid(T));
        detach_component_user(type_index, id);
    }

    template<ResourceConcept R>
    void Rc<R>::detach_component_user(const std::type_index &ti, const EntityID id) const {
        if (!component_users->contains(ti))
            return;
        component_users->at(ti).erase(id);
    }

    template<ResourceConcept R>
    template<ResourceConcept T>
    void Rc<R>::attach_resource_user(const std::string &name) const {
        const auto &type_index = std::type_index(typeid(T));
        attach_resource_user(type_index, name);
    }

    template<ResourceConcept R>
    void Rc<R>::attach_resource_user(const std::type_index &ti, const std::string &name) const {
        if (!resource_users->contains(ti)) {
            resource_users->insert_or_assign(ti, std::unordered_set<std::string>());
        }
        resource_users->at(ti).insert(name);
    }

    template<ResourceConcept R>
    template<ResourceConcept T>
    void Rc<R>::detach_resource_user(const std::string &name) const {
        const auto &type_index = std::type_index(typeid(T));
        detach_resource_user(type_index, name);
    }

    template<ResourceConcept R>
    void Rc<R>::detach_resource_user(const std::type_index &ti, const std::string &name) const {
        if (!resource_users->contains(ti))
            return;
        resource_users->at(ti).erase(name);
    }

    template<ResourceConcept R>
    template<ComponentConcept T>
    const std::unordered_set<EntityID> &Rc<R>::get_users() const {
        const auto &type_index = std::type_index(typeid(T));
        if (!component_users->contains(type_index))
            component_users->insert_or_assign(type_index, std::unordered_set<EntityID>());

        return component_users->at(type_index);
    }

    template<ResourceConcept R>
    template<ResourceConcept T>
    const std::unordered_set<std::string> &Rc<R>::get_users() const {
        const auto &type_index = std::type_index(typeid(T));
        if (!resource_users->contains(type_index))
            resource_users->insert_or_assign(type_index, std::unordered_set<std::string>());

        return resource_users->at(type_index);
    }

    template<ResourceConcept R>
    template<ComponentConcept T>
    std::vector<EntityID> Rc<R>::get_common_users(const std::vector<std::shared_ptr<const T>> &list) const {
        const auto &type_index = std::type_index(typeid(T));
        if (!component_users->contains(type_index))
            return {};

        std::vector<EntityID> res{};
        res.reserve(list.size());

        for (const auto &e: list) {
            if (component_users->at(type_index).contains(e->get_entity())) {
                res.push_back(e.get()->get_entity());
            }
        }

        return res;
    }

    template<ResourceConcept R>
    template<ResourceConcept T>
    std::vector<std::string> Rc<R>::get_common_users(const std::vector<Rc<T>> &list) const {
        const auto &type_index = std::type_index(typeid(T));
        if (!resource_users->contains(type_index))
            return {};

        std::vector<std::string> res{};
        res.reserve(list.size());

        for (const auto &e: list) {
            const std::string &name = e.get()->get_name();
            if (resource_users->at(type_index).contains(name)) {
                res.push_back(name);
            }
        }

        return res;
    }

    template<ResourceConcept R>
    template<ResourceConcept T>
    Rc<T> Rc<R>::as() const {
        auto casted = std::dynamic_pointer_cast<T>(resource);
        if (!casted) {
            throw std::bad_cast();
        }
        return Rc<T>(casted, component_users, resource_users);
    }

    template<ResourceConcept R>
    template<ResourceConcept T>
    Rc<T> *Rc<R>::as_ptr() {
        return reinterpret_cast<Rc<T> *>(this);
    }

    // template<ResourceConcept R>
    // template<ResourceConcept T>
    // std::shared_ptr<Rc<T>> Rc<R>::as_ptr() const {
    //     auto casted = std::dynamic_pointer_cast<T>(resource);
    //     if (!casted) {
    //         throw std::bad_cast();
    //     }
    //     // Wrap the Rc<T> in a shared_ptr
    //     return std::make_shared<Rc<T>>(casted, component_users, resource_users);
    // }


} // namespace wrld
