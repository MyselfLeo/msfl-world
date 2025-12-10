//
// Created by leo on 9/8/25.
//

#include <wrld/builtins.hpp>

#include <wrld/components/StaticModel.hpp>
#include <wrld/components/Transform.hpp>
#include <../../include/wrld/objects/geometry/Mesh.hpp>
#include <wrld/Main.hpp>
#include <wrld/components/Camera3D.hpp>
#include <wrld/components/FPSControl.hpp>
#include <wrld/resources/Model.hpp>

namespace wrld::builtins {
    Rc<rsc::Material> unlit_material(World &world) {
        static bool set = false;
        static Rc<rsc::Material> ptr;

        if (!set) {
            ptr = world.create_resource<rsc::Material>("unlit_material");
            ptr->do_lighting(false);
            ptr->set_line_width(2);
            set = true;
        }

        return ptr;
    }

    EntityID create_grid(World &world, const unsigned nb_lines,
                         const float line_spacing) {
        // Create the mesh
        auto vertex = obj::Vertex({0, 0, 0}, {1, 0, 0}, {0, 0}, {0.7, 0.7, 0.7});

        float offset = line_spacing * (static_cast<float>(nb_lines) / 2.0f);
        if (nb_lines % 2 == 0) {
            offset -= line_spacing / 2.0f;
        }

        // Create vertices
        std::vector<obj::Vertex> vertices;
        std::vector<obj::VertexID> elements;
        vertices.reserve(nb_lines * 4);
        elements.reserve(nb_lines * 4);

        // X direction
        for (int i = 0; i < nb_lines; i++) {
            vertex.position = {-offset + line_spacing * static_cast<float>(i), 0, offset};
            vertices.push_back(vertex);
            vertex.position = {-offset + line_spacing * static_cast<float>(i), 0,
                               -offset};
            vertices.push_back(vertex);
        }

        // Y direction
        for (int i = 0; i < nb_lines; i++) {
            vertex.position = {offset, 0, -offset + line_spacing * static_cast<float>(i)};
            vertices.push_back(vertex);
            vertex.position = {-offset, 0,
                               -offset + line_spacing * static_cast<float>(i)};
            vertices.push_back(vertex);
        }

        for (int i = 0; i < nb_lines * 4; i++) {
            elements.push_back(i);
        }

        // Create model
        obj::Mesh mesh;
        mesh.set_vertices(vertices).set_elements(elements).set_primitive_type(
                obj::PrimitiveType::Lines);

        Rc<rsc::Model> model = world.create_resource<rsc::Model>("grid_model");
        model->from_mesh(mesh, unlit_material(world));

        // Create entity
        const EntityID entity = world.create_entity("Grid");
        world.attach_component<cpt::Transform>(entity);
        world.attach_component<cpt::StaticModel>(entity, model);

        return entity;
    }

    EntityID create_axis(World &world, float axis_length) {
        // Create the mesh

        std::vector<obj::Vertex> vertices;
        std::vector<obj::VertexID> elements;
        vertices.reserve(6); // 3 lines so 6 vertices
        elements.reserve(6);

        // X => R
        vertices.push_back(obj::Vertex({0, 0, 0}, {1, 1, 1}, {0, 0}, {1, 0, 0}));
        vertices.push_back(
                obj::Vertex({axis_length, 0, 0}, {1, 1, 1}, {0, 0}, {1, 0, 0}));

        // Y => G
        vertices.push_back(obj::Vertex({0, 0, 0}, {1, 1, 1}, {0, 0}, {0, 1, 0}));
        vertices.push_back(
                obj::Vertex({0, axis_length, 0}, {1, 1, 1}, {0, 0}, {0, 1, 0}));

        // Z => B
        vertices.push_back(obj::Vertex({0, 0, 0}, {1, 1, 1}, {0, 0}, {0, 0, 1}));
        vertices.push_back(
                obj::Vertex({0, 0, axis_length}, {1, 1, 1}, {0, 0}, {0, 0, 1}));

        for (int i = 0; i < 6; i++) {
            elements.push_back(i);
        }

        // Create model
        obj::Mesh mesh;
        mesh.set_vertices(vertices).set_elements(elements).set_primitive_type(
                obj::PrimitiveType::Lines);

        Rc<rsc::Model> model = world.create_resource<rsc::Model>("axis_model");
        model->from_mesh(mesh, unlit_material(world));

        // Create entity
        const EntityID entity = world.create_entity("Axis");
        world.attach_component<cpt::Transform>(entity);
        world.attach_component<cpt::StaticModel>(entity, model);

        return entity;
    }

    EntityID flyover(World &world) {
        const EntityID flyover = world.create_entity("Flyover");

        world.attach_component<cpt::Transform>(flyover);
        world.attach_component<cpt::FPSControl>(flyover);
        world.attach_component<cpt::Camera3D>(flyover, 45, true,
                                              Main::get_window_viewport(),
                                              world.get_default<rsc::Program>());

        const auto &env = world.attach_component<cpt::Environment3D>(flyover);
        env->set_ambiant_light(cpt::AmbiantLight{glm::vec3{1.0}, 1.0});

        return flyover;
    }

    obj::Mesh cube() {
        obj::Mesh mesh(obj::PrimitiveType::Triangles);

        // Common colors (White)
        glm::vec3 c_white(1.f, 1.f, 1.f);

        // Define the 6 faces (Position, Normal, UVs)
        // Order: Front, Back, Right, Left, Top, Bottom
        const std::vector<obj::Vertex> vertices = {
                // -- Front Face (Normal +Z) --
                {{-0.5, -0.5, 0.5}, {0, 0, 1}, {0, 0}, c_white}, // Bottom-Left
                {{0.5, -0.5, 0.5}, {0, 0, 1}, {1, 0}, c_white}, // Bottom-Right
                {{0.5, 0.5, 0.5}, {0, 0, 1}, {1, 1}, c_white}, // Top-Right
                {{-0.5, 0.5, 0.5}, {0, 0, 1}, {0, 1}, c_white}, // Top-Left

                // -- Back Face (Normal -Z) --
                {{0.5, -0.5, -0.5},
                 {0, 0, -1},
                 {0, 0},
                 c_white}, // Bottom-Left (relative to back view)
                {{-0.5, -0.5, -0.5}, {0, 0, -1}, {1, 0}, c_white}, // Bottom-Right
                {{-0.5, 0.5, -0.5}, {0, 0, -1}, {1, 1}, c_white}, // Top-Right
                {{0.5, 0.5, -0.5}, {0, 0, -1}, {0, 1}, c_white}, // Top-Left

                // -- Right Face (Normal +X) --
                {{0.5, -0.5, 0.5}, {1, 0, 0}, {0, 0}, c_white},
                {{0.5, -0.5, -0.5}, {1, 0, 0}, {1, 0}, c_white},
                {{0.5, 0.5, -0.5}, {1, 0, 0}, {1, 1}, c_white},
                {{0.5, 0.5, 0.5}, {1, 0, 0}, {0, 1}, c_white},

                // -- Left Face (Normal -X) --
                {{-0.5, -0.5, -0.5}, {-1, 0, 0}, {0, 0}, c_white},
                {{-0.5, -0.5, 0.5}, {-1, 0, 0}, {1, 0}, c_white},
                {{-0.5, 0.5, 0.5}, {-1, 0, 0}, {1, 1}, c_white},
                {{-0.5, 0.5, -0.5}, {-1, 0, 0}, {0, 1}, c_white},

                // -- Top Face (Normal +Y) --
                {{-0.5, 0.5, 0.5}, {0, 1, 0}, {0, 0}, c_white},
                {{0.5, 0.5, 0.5}, {0, 1, 0}, {1, 0}, c_white},
                {{0.5, 0.5, -0.5}, {0, 1, 0}, {1, 1}, c_white},
                {{-0.5, 0.5, -0.5}, {0, 1, 0}, {0, 1}, c_white},

                // -- Bottom Face (Normal -Y) --
                {{-0.5, -0.5, -0.5}, {0, -1, 0}, {0, 0}, c_white},
                {{0.5, -0.5, -0.5}, {0, -1, 0}, {1, 0}, c_white},
                {{0.5, -0.5, 0.5}, {0, -1, 0}, {1, 1}, c_white},
                {{-0.5, -0.5, 0.5}, {0, -1, 0}, {0, 1}, c_white}};

        std::vector<obj::VertexID> indices;
        indices.reserve(36);

        for (int i = 0; i < 6; ++i) {
            const obj::VertexID offset = i * 4;
            // First Triangle
            indices.push_back(offset + 0);
            indices.push_back(offset + 1);
            indices.push_back(offset + 2);
            // Second Triangle
            indices.push_back(offset + 2);
            indices.push_back(offset + 3);
            indices.push_back(offset + 0);
        }

        mesh.set_vertices(vertices);
        mesh.set_elements(indices);

        return mesh;
    }
} // namespace wrld::builtins
