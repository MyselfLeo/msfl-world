//
// Created by leo on 9/6/25.
//

#ifndef RESOURCE_HPP
#define RESOURCE_HPP

// ReSharper disable once CppUnusedIncludeDirective
#include <cstddef>

namespace wrld {
    // Define World beforehand. We can't #include "../World.hpp" as it would
    // create a circular reference.
    class World;
} // namespace wrld

namespace wrld::rsc {
    typedef size_t ResourceID;

    /// Base class for all resources.
    /// Any derived class must possess a constructor expecting ResourceID as its first parameter (in order to initialise
    /// the base class). This constructor will be called by World::create_resource.

    class Resource {
    public:
        virtual ~Resource() = default;
        explicit Resource(ResourceID resource_id);

        [[nodiscard]] ResourceID get_id() const;

    protected:
        ResourceID resource_id;
        // World &world;
    };

} // namespace wrld::rsc

#endif // RESOURCE_HPP
