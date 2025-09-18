//
// Created by leo on 9/6/25.
//

#ifndef RESOURCE_HPP
#define RESOURCE_HPP

#include "concepts.hpp"

#include "../components/Component.hpp"

#include <memory>
#include <ranges>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include <utility>
#include <format>

// Forward declaration
namespace wrld {
    class Component;
    class World;
} // namespace wrld

namespace wrld {
    /// Base class for all resources.
    /// Any derived class must possess a constructor expecting std::string and World& as its firt
    /// AND ONLY parameters. This constructor will be called by World::create_resource.
    /// Any parameter should have default values that can be changed by setters afterward.
    /// Why : This allows the framework to create "default" Resources of each type, accessible via
    /// world.get_default<>().
    /// Furthermore : TODO

    class Resource : public std::enable_shared_from_this<Resource> {
    public:
        virtual ~Resource() = default;
        explicit Resource(std::string name, World &world);

        // [[nodiscard]] ResourceID get_id() const;
        [[nodiscard]] std::string get_name() const;

        virtual std::string get_type() { return "Resource"; }

    protected:
        friend class Component;

        // ResourceID resource_id;
        std::string name;
        World &world;

        void attach_component_user(const std::shared_ptr<const Component> &user);
        void detach_component_user(const Component *user);
        void attach_resource_user(const std::shared_ptr<const Resource> &user);
        void detach_resource_user(const Resource *user);

        template<typename T>
        std::vector<std::shared_ptr<const T>> get_users() {
            // If T is a resource
            if constexpr (ResourceConcept<T>) {
                std::vector<std::shared_ptr<const Resource>> res;
                for (const auto &v: resource_users[std::type_index(typeid(T))] | std::views::values) {
                    res.push_back(v);
                }

                return res;
            }
            // If T is a component
            else if constexpr (ComponentConcept<T>) {
                std::vector<std::shared_ptr<const Component>> res;
                for (const auto &v: component_users[std::type_index(typeid(T))] | std::views::values) {
                    res.push_back(v);
                }

                return res;
            }

            std::unreachable();
        }

        void attach_resource(const std::string &unique_name, const std::shared_ptr<Resource> &resource);

        template<ResourceConcept R>
        std::shared_ptr<const R> get_resource(const std::string &unique_name) {
            if (!attached_resources.contains(unique_name))
                throw std::runtime_error(std::format("Tried to access unbound resource {}", unique_name));

            return std::dynamic_pointer_cast<const R>(attached_resources[unique_name]);
        }

        void detach_resource(const std::string &unique_name) {
            if (!attached_resources.contains(unique_name))
                return;

            attached_resources[unique_name]->detach_resource_user(this);
        }

        /// Resources attached to this component.
        /// We don't use shared_ptr as direct members of the class because we'd like to
        /// track components attached to each resource.
        std::unordered_map<std::string, std::shared_ptr<Resource>> attached_resources;

    private:
        /// List of components using this resource.
        std::unordered_map<std::type_index, std::unordered_map<const Component *, std::shared_ptr<const Component>>>
                component_users;
        /// List of resources using this resource.
        std::unordered_map<std::type_index, std::unordered_map<const Resource *, std::shared_ptr<const Resource>>>
                resource_users;
    };

} // namespace wrld

#endif // RESOURCE_HPP
