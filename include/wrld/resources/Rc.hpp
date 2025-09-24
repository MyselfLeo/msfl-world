//
// Created by leo on 9/19/25.
//

#ifndef RC_HPP
#define RC_HPP

#include <memory>
#include <unordered_map>
#include <vector>
#include <ranges>
#include <typeindex>

#include <wrld/concepts.hpp>

#include <unordered_set>

namespace wrld {
    class World;
    class Resource;
    class Component;
    typedef size_t EntityID;

    /// ResourceCounter
    template<ResourceConcept R>
    class Rc {
        typedef std::unordered_map<std::type_index, std::unordered_set<EntityID> > UserComponentPool;
        typedef std::unordered_map<std::type_index, std::unordered_set<std::string> > UserResourcePool;

    public:
        Rc();

        Rc(std::string name, World &world);

        Rc(std::shared_ptr<R> ptr, std::shared_ptr<UserComponentPool> comp_users,
           std::shared_ptr<UserResourcePool> res_users);

        const R *operator->() const;

        R *operator->();

        const R *get() const;

        R *get_mut() const;

        std::shared_ptr<R> get_ptr() const;

        R &get_ref() const;

        template<ComponentConcept T>
        void attach_component_user(EntityID id) const;

        void attach_component_user(const std::type_index &ti, EntityID id) const;

        template<ComponentConcept T>
        void detach_component_user(EntityID id) const;

        void detach_component_user(const std::type_index &ti, EntityID id) const;

        template<ResourceConcept T>
        void attach_resource_user(const std::string &name) const;

        void attach_resource_user(const std::type_index &ti, const std::string &name) const;

        template<ResourceConcept T>
        void detach_resource_user(const std::string &name) const;

        void detach_resource_user(const std::type_index &ti, const std::string &name) const;

        template<ComponentConcept T>
        const std::unordered_set<EntityID> &get_users() const;

        template<ResourceConcept T>
        const std::unordered_set<std::string> &get_users() const;

        template<ComponentConcept T>
        std::vector<EntityID> get_common_users(const std::vector<std::shared_ptr<const T> > &list) const;

        template<ResourceConcept T>
        std::vector<std::string> get_common_users(const std::vector<Rc<T> > &list) const;

        template<ResourceConcept T>
        Rc<T> as() const;

        template<ResourceConcept T>
        Rc<T> *as_ptr();

    private:
        std::shared_ptr<R> resource;
        std::shared_ptr<UserComponentPool> component_users;
        std::shared_ptr<UserResourcePool> resource_users;
    };
} // namespace wrld

#include <wrld/resources/Rc.tpp>

#endif // RC_HPP
