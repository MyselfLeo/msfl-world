//
// Created by leo on 8/11/25.
//

#ifndef COMPONENT_HPP
#define COMPONENT_HPP

#include <cstddef>
#include <tuple>

namespace wrld {
    typedef size_t EntityID;
    class World;

    /// Base class for all components.
    /// Any derived class must possess a constructor expecting EntityID and World& as their
    /// first two parameters (in order to initialise the base class).
    /// This constructor will be called by World::attach_component.
    class Component {
    public:
        virtual ~Component() = default;
        Component(EntityID entity_id, World &world);

    private:
        EntityID entity_id;
        World &world;
    };

} // namespace wrld

#endif // COMPONENT_HPP
