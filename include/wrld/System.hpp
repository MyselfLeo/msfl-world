//
// Created by leo on 8/16/25.
//

#ifndef SYSTEM_HPP
#define SYSTEM_HPP
#include <wrld/World.hpp>

namespace wrld {

    class System {
    public:
        explicit System(World &world);

        virtual ~System() = default;

        /// Called by the event-loop on each frame.
        virtual void exec() = 0;

    protected:
        World &world;
    };

} // namespace wrld

#endif // SYSTEM_HPP
