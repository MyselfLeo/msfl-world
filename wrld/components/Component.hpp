//
// Component.hpp
//

#ifndef COMPONENT_HPP
#define COMPONENT_HPP

#include <format>
#include <memory>
#include <string>
#include <unordered_map>

#include "concepts.hpp"

namespace wrld {
    class Resource;
    class World;
    typedef size_t EntityID;

    template<ResourceConcept>
    class Rc;

    class Component : public std::enable_shared_from_this<Component> {
    public:
        virtual ~Component() = default;
        Component(EntityID entity_id, World &world);

        [[nodiscard]] EntityID get_entity() const;

        virtual std::string get_type() { return "Component"; }

        // todo: move to a higher class common with component
        // virtual void load_default_resources() = 0;

    protected:
        EntityID entity_id;
        World &world;

        template<ResourceConcept R>
        void attach_resource(const std::string &unique_name, const Rc<R> &resource);

        template<ResourceConcept R>
        Rc<R> get_resource(const std::string &unique_name) const;

        void detach_resource(const std::string &unique_name);

        bool has_resource(const std::string &unique_name) const;

        std::unordered_map<std::string, Rc<Resource>> attached_resources;
    };
} // namespace wrld

// 👇 Implementation
#include "Component.tpp"

#endif // COMPONENT_HPP
