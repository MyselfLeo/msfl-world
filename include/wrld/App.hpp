//
// Created by leo on 9/11/25.
//

#ifndef APP_HPP
#define APP_HPP
#include <wrld/World.hpp>

namespace wrld {
    class App {
    public:
        virtual ~App() = default;

        virtual void init(World &world) = 0;
        virtual void update(World &world, double deltatime) = 0;
        virtual void ui(World &world) = 0;
        virtual void exit(World &world) = 0;
    };

} // namespace wrld

#endif // APP_HPP
