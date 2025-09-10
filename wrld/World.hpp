//
// Created by leo on 8/11/25.
//

#ifndef WORLD_HPP
#define WORLD_HPP
#include "components/Component.hpp"
#include "resources/Resource.hpp"

#include <format>
#include <memory>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <vector>

namespace wrld {
    typedef size_t EntityID;
    typedef std::unordered_map<std::type_index, std::unordered_map<std::string, std::shared_ptr<rsc::Resource>>>
            ResourcePool;
    typedef std::unordered_map<std::type_index, std::unordered_map<EntityID, std::shared_ptr<cpt::Component>>>
            ComponentPool;


    /// Concept of a Component: it must inherit the cpt::Component class
    template<class T>
    concept Component = std::is_base_of_v<cpt::Component, T>;

    /// Concept of a Resource: it must inherit the rsc::Resource class
    template<class T>
    concept Resource = std::is_base_of_v<rsc::Resource, T>;

    class World {
    public:
        World();

        /// Creates an Entity, returning its ID. An optional name can be given. It doesn't need to be unique.
        EntityID create_entity(const std::string &name = "");

        std::string get_entity_name(EntityID id);

        /// Delete the Entity and all attached Components.
        void delete_entity(EntityID id);

        std::unordered_map<EntityID, std::string> get_entities() const;

        /// Attach a new component of the given type to the entity,
        /// returning a reference to it.
        template<Component C, typename... Args>
        std::shared_ptr<C> attach_component(const EntityID id, Args &&...args) {
            if (!entity_exists(id))
                throw std::runtime_error("Creating a Component on inexisting Entity");

            if (!components.contains(std::type_index(typeid(C)))) {
                components[std::type_index(typeid(C))] = {};
            }

            if (components[std::type_index(typeid(C))].contains(id))
                throw std::runtime_error("The entity already has a component of this type.");

            // Returns the created component
            auto new_comp = std::make_shared<C>(id, *this, std::forward<Args>(args)...);
            components[std::type_index(typeid(C))][id] = new_comp;
            return new_comp;
        }

        /// Returns an optional pointer to the component of the given type
        /// attached to the given object.
        template<Component C>
        std::optional<std::shared_ptr<C>> get_component_opt(const EntityID id) {
            if (!components.contains(std::type_index(typeid(C))))
                return std::nullopt;
            if (!components[std::type_index(typeid(C))].contains(id))
                return std::nullopt;

            return static_pointer_cast<C>(components[std::type_index(typeid(C))][id]);
        }

        /// Returns a pointer to the component of the given type attached to the
        /// given object. Throws std::runtime_error if no component of this type
        /// is attached to the object.
        template<Component C>
        std::shared_ptr<C> get_component(const EntityID id) {
            if (!components.contains(std::type_index(typeid(C))))
                throw std::runtime_error(
                        std::format("Entity {} does not have a component {} attached to it", id, typeid(C).name()));
            if (!components[std::type_index(typeid(C))].contains(id))
                throw std::runtime_error(
                        std::format("Entity {} does not have a component {} attached to it", id, typeid(C).name()));

            return static_pointer_cast<C>(components[std::type_index(typeid(C))][id]);
        }

        /// Return a vector of entities that have the given type
        /// of component attached to them.
        template<Component C>
        std::vector<EntityID> get_entities_with_component() {
            std::vector<EntityID> res;
            res.reserve(components[std::type_index(typeid(C))].size());

            for (const auto k: components[std::type_index(typeid(C))] | std::views::keys) {
                res.push_back(k);
            }

            return res;
        }

        /// Return all components type attached to the entity.
        // todo: maybe store thoses maps to accelerate this
        std::vector<std::type_index> get_components_of_entity(EntityID id) const;

        template<Resource R, typename... Args>
        std::shared_ptr<R> create_resource(const std::string &name, Args &&...args) {
            // // Creates a new id
            // max_resource_id += 1;
            // rsc::ResourceID id = max_resource_id;

            // Create unique name from given name
            std::string new_name = name;
            while (resources[std::type_index(typeid(R))].contains(new_name)) {
                new_name = std::format("{}:{}", name, generate_random_id());
            }

            // Returns the created resource
            auto new_ressource = std::make_shared<R>(new_name, *this, std::forward<Args>(args)...);

            if (!resources.contains(std::type_index(typeid(R)))) {
                resources[std::type_index(typeid(R))] = {};
            }

            resources[std::type_index(typeid(R))][new_name] = new_ressource;
            return new_ressource;
        }

        const ResourcePool &get_resources() const;

    private:
        friend class System;

        // Stores Entity names
        unsigned max_entity_id = 0;
        std::unordered_map<EntityID, std::string> entities;

        // Access a resource by its resource ID.
        unsigned max_resource_id = 0;
        ResourcePool resources;

        // Access a component first by type then by entity ID.
        // Ensure that two components of the same type cannot be applied to the same entity.
        ComponentPool components;

        static size_t generate_random_id();

        /// Returns true if the given entity id exists in this world.
        bool entity_exists(EntityID id) const;
    };
} // namespace wrld

#endif // WORLD_HPP
