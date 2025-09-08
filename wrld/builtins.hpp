//
// Created by leo on 9/8/25.
//

#ifndef BUILTINS_HPP
#define BUILTINS_HPP
#include "World.hpp"

namespace wrld::builtins {
    /// Creates an Entity with a grid model and a default transform.
    EntityID create_grid(World &world, unsigned nb_lines = 10, float line_spacing = 1.0);

    /// Creates an Entity with an axis model and a default transform.
    EntityID create_axis(World &world, float axis_length = 1.0);
} // namespace wrld::builtins

#endif // BUILTINS_HPP
