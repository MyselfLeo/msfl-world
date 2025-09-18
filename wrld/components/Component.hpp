//
// Created by leo on 8/11/25.
//

#ifndef COMPONENT_HPP
#define COMPONENT_HPP

#include <format>
#include <memory>
#include <string>
#include <unordered_map>

#include "concepts.hpp"

namespace wrld {
    class Resource; // Forward declaration
    class World;
    typedef size_t EntityID;
} // namespace wrld

namespace wrld {
    typedef size_t EntityID;

    /// Base class for all components.
    /// Any derived class must possess a constructor expecting EntityID and World& as their
    /// first two parameters (in order to initialise the base class).
    /// This constructor will be called by World::attach_component.
    class Component : public std::enable_shared_from_this<Component> {
    public:
        virtual ~Component() = default;
        Component(EntityID entity_id, World &world);

        [[nodiscard]] EntityID get_entity() const;

        /// Return the name of the Component type
        virtual std::string get_type() { return "Component"; }

    protected:
        EntityID entity_id;
        World &world;

        void attach_resource(const std::string &unique_name, const std::shared_ptr<Resource> &resource);

        template<ResourceConcept R>
        std::shared_ptr<const R> get_resource(const std::string &unique_name) {
            if (!attached_resources.contains(unique_name))
                throw std::runtime_error(std::format("Tried to access unbound resource {}", unique_name));

            return std::dynamic_pointer_cast<const R>(attached_resources[unique_name]);
        }

        void detach_resource(const std::string &unique_name);

        /// Resources attached to this component.
        /// We don't use shared_ptr as direct members of the class because we'd like to
        /// track components attached to each resource.
        std::unordered_map<std::string, std::shared_ptr<Resource>> attached_resources;
    };

} // namespace wrld

#endif // COMPONENT_HPP
