//
// Created by leo on 9/8/25.
//

#pragma once

#include <wrld/World.hpp>
#include <wrld/objects/geometry/Mesh.hpp>
#include <wrld/resources/Material.hpp>

namespace wrld::builtins {
    /// Returns the default material without lighting.
    Rc<rsc::Material> unlit_material(World &world);

    /// Creates an Entity with a grid model and a default transform.
    EntityID create_grid(World &world, unsigned nb_lines = 10, float line_spacing = 1.0);

    /// Creates an Entity with an axis model and a default transform.
    EntityID create_axis(World &world, float axis_length = 1.0);

    /// Create an Entity with a Camera3D, a FPSControl, and an Env componant.
    EntityID flyover(World &world);

    /// Return a cube mesh of size 1.
    obj::Mesh cube();
} // namespace wrld::builtins
