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
    /// You SHOULD NOT override
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
