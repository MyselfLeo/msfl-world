//
// Created by leo on 9/6/25.
//

#pragma once

#include <memory>
#include <string>
#include <format>
#include <iostream>
#include <unordered_map>

#include <wrld/concepts.hpp>
#include <wrld/logs.hpp>

namespace wrld {
    class Component;
    class World;

    template<ResourceConcept>
    class Rc;

    class Resource : public std::enable_shared_from_this<Resource> {
    public:
        virtual ~Resource() { wrldInfo(std::format("Resource '{}' got freed", get_name())); }

        explicit Resource(std::string name, World &world /*, Rc<Resource> *rc*/);

        // todo: move to a higher class common with component
        // todo: DO THE SAME FOR COMPONENTS
        // virtual void load_default_resources() = 0;

        [[nodiscard]] std::string get_name() const;

        virtual std::string get_type() const { return "Resource"; }

    protected:
        friend class Rc<Resource>;
        friend class Component;
        std::string name;
        World &world;
        // Rc<Resource> *rc;

        /// Attach a resource R to this object.
        /// This resource will be later accessible via get_resource.
        template<ResourceConcept R>
        void attach_resource(const std::string &unique_name, const Rc<R> &resource);

        template<ResourceConcept R>
        Rc<R> get_resource(const std::string &unique_name) const;

        void detach_resource(const std::string &unique_name);

        bool has_resource(const std::string &unique_name) const;

        std::unordered_map<std::string, Rc<Resource>> attached_resources;
    };
} // namespace wrld

#include <wrld/resources/Resource.tpp>
