//
// Created by leo on 8/11/25.
//

#pragma once

#include <wrld/components/Component.hpp>
#include <wrld/resources/Resource.hpp>
#include <wrld/resources/Rc.hpp>

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
    typedef std::unordered_map<std::type_index, std::unordered_map<std::string, Rc<Resource>>> ResourcePool;
    typedef std::unordered_map<std::type_index, Rc<Resource>> DefaultResourcePool;
    typedef std::unordered_map<std::type_index, std::unordered_map<EntityID, std::shared_ptr<Component>>> ComponentPool;

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
        template<ComponentConcept C, typename... Args>
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
        template<ComponentConcept C>
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
        template<ComponentConcept C>
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
        template<ComponentConcept C>
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

        template<ResourceConcept R>
        Rc<R> create_resource(const std::string &name) {
            if (!resources.contains(std::type_index(typeid(R)))) {
                resources[std::type_index(typeid(R))] = {};
            }

            // Create unique name from given name
            std::string new_name = name;
            while (resources.at(std::type_index(typeid(R))).contains(new_name)) {
                new_name = std::format("{}:{}", name, generate_random_id());
            }

            // Returns the created resource
            Rc<R> new_ressource = Rc<R>(new_name, *this);

            if (!resources.contains(std::type_index(typeid(R)))) {
                resources[std::type_index(typeid(R))] = {};
            }

            resources.at(std::type_index(typeid(R))).insert_or_assign(new_name, new_ressource.template as<Resource>());
            return new_ressource;
        }

        /// Return the resource of the given type with the given name.
        /// Throws std::runtime_error if no such resource exists.
        template<ResourceConcept R>
        Rc<R> get_resource(const std::string &name) {
            if (!resources.contains(std::type_index(typeid(R))))
                throw std::runtime_error("This resource does not exists");

            if (!resources.at(std::type_index(typeid(R))).contains(name))
                throw std::runtime_error("This resource does not exists");

            return resources.at(std::type_index(typeid(R)))[name].as<R>();
        }

        template<ResourceConcept R>
        Rc<R> get_default() {
            if (!default_resources.contains(std::type_index(typeid(R)))) {
                Rc<R> new_resource = Rc<R>("default", *this);
                const Rc<Resource> casted = new_resource.template as<Resource>();

                default_resources.insert_or_assign(std::type_index(typeid(R)), casted);
            }

            return default_resources.at(std::type_index(typeid(R))).as<R>();
        }

        const ResourcePool &get_resources() const;

    private:
        friend class System;

        // Stores Entity names
        unsigned max_entity_id = 0;
        std::unordered_map<EntityID, std::string> entities;

        // Default resources, one for each type. They are immutable.
        DefaultResourcePool default_resources;

        // Access a resource by its name. World::create_resource ensure that this name is unique.
        ResourcePool resources;

        // Access a component first by type then by entity ID.
        // Ensure that two components of the same type cannot be applied to the same entity.
        ComponentPool components;

        static size_t generate_random_id();

        /// Returns true if the given entity id exists in this world.
        bool entity_exists(EntityID id) const;
    };
} // namespace wrld
