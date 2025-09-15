//
// Created by leo on 9/6/25.
//

#ifndef RESOURCE_HPP
#define RESOURCE_HPP

// ReSharper disable once CppUnusedIncludeDirective
#include <cstddef>
#include <memory>
#include <string>

namespace wrld {
    // Define World beforehand. We can't #include "../World.hpp" as it would
    // create a circular reference.
    class World;
} // namespace wrld

namespace wrld::rsc {
    /// Base class for all resources.
    /// Any derived class must possess a constructor expecting std::string and World& as its firt
    /// AND ONLY parameters. This constructor will be called by World::create_resource.
    /// Any parameter should have default values that can be changed by setters afterward.
    /// Why : This allows the framework to create "default" Resources of each type, accessible via
    /// world.get_default<>().
    /// Furthermore : TODO

    class Resource {
    public:
        virtual ~Resource() = default;
        explicit Resource(std::string name, World &world);

        // [[nodiscard]] ResourceID get_id() const;
        [[nodiscard]] std::string get_name() const;

        virtual std::string get_type() { return "Resource"; }

    protected:
        // ResourceID resource_id;
        std::string name;
        World &world;
    };

} // namespace wrld::rsc

#endif // RESOURCE_HPP
