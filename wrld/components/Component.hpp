//
// Created by leo on 8/11/25.
//

#ifndef COMPONENT_HPP
#define COMPONENT_HPP

// ReSharper disable once CppUnusedIncludeDirective
#include <cstddef>
#include <string>

namespace wrld {
    // Define World beforehand. We can't #include "../World.hpp" as it would
    // create a circular reference.
    class World;
} // namespace wrld

namespace wrld::cpt {
    typedef size_t EntityID;

    /// Base class for all components.
    /// Any derived class must possess a constructor expecting EntityID and World& as their
    /// first two parameters (in order to initialise the base class).
    /// This constructor will be called by World::attach_component.
    class Component {
    public:
        virtual ~Component() = default;
        Component(EntityID entity_id, World &world);

        [[nodiscard]] EntityID get_entity() const;

        /// Return the name of the Component type
        virtual std::string get_type() { return "Component"; }

    protected:
        EntityID entity_id;
        World &world;
    };

} // namespace wrld::cpt

#endif // COMPONENT_HPP
