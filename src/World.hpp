//
// Created by leo on 8/11/25.
//

#ifndef WORLD_HPP
#define WORLD_HPP
#include "Component.hpp"

#include <memory>
#include <optional>
#include <stdexcept>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>

namespace wrld {
    typedef size_t EntityID;

    class World {
    public:
        World();

        /// Creates an Entity, returning its ID.
        EntityID create_entity();

        /// Delete the Entity and all attached Components.
        void delete_entity(EntityID id);

        /// Attach a new component of the given type to the entity,
        /// returning a reference to it.
        template<typename T>
        std::shared_ptr<T> attach_component(const EntityID id) {
            static_assert(std::is_base_of_v<Component, T>, "The Component's type must inherit Component");

            if (!exists(id))
                throw std::runtime_error("Creating a Component on inexisting Entity");

            if (!components.contains(std::type_index(typeid(T)))) {
                components[std::type_index(typeid(T))] = {};
            }

            if (components[std::type_index(typeid(T))].contains(id))
                throw std::runtime_error("The entity already has a component of this type.");

            // Returns the created component
            auto new_comp = std::make_shared<T>(id, *this);
            components[std::type_index(typeid(T))][id] = new_comp;
            return new_comp;
        }

        /// Returns a reference to the component of the given type
        /// attached to the given object, if it exists.
        template<typename T>
        std::optional<std::shared_ptr<T>> get_component(const EntityID id) {
            if (!components.contains(std::type_index(typeid(T))))
                return std::nullopt;
            if (!components[std::type_index(typeid(T))].contains(id))
                return std::nullopt;

            return static_pointer_cast<T>(components[std::type_index(typeid(T))][id]);
        }

        /// Returns true if the given entity id exists in this world.
        bool exists(EntityID id) const;

    private:
        std::unordered_set<EntityID> entities;

        // Access a component first by type then by entity ID.
        // Ensure that two components of the same type cannot be applied to the same entity.
        std::unordered_map<std::type_index, std::unordered_map<EntityID, std::shared_ptr<Component>>> components;
    };
} // namespace wrld

#endif // WORLD_HPP
