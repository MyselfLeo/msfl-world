//
// Created by leo on 9/8/25.
//

#include "builtins.hpp"

#include "components/StaticModel.hpp"
#include "components/Transform.hpp"
#include "resources/Mesh.hpp"
#include "resources/Model.hpp"

namespace wrld::builtins {
    EntityID create_grid(World &world, const unsigned nb_lines, const float line_spacing) {
        // Create the mesh
        // todo: make it not lighted
        auto vertex = rsc::Vertex({0, 0, 0}, {1, 0, 0}, {0, 0}, {0.7, 0.7, 0.7});

        float offset = line_spacing * (static_cast<float>(nb_lines) / 2.0f);
        if (nb_lines % 2 == 0) {
            offset -= line_spacing / 2.0f;
        }

        // Create vertices
        std::vector<rsc::Vertex> vertices;
        std::vector<rsc::VertexID> elements;
        vertices.reserve(nb_lines * 4);
        elements.reserve(nb_lines * 4);

        // X direction
        for (int i = 0; i < nb_lines; i++) {
            vertex.position = {-offset + line_spacing * static_cast<float>(i), 0, offset};
            vertices.push_back(vertex);
            vertex.position = {-offset + line_spacing * static_cast<float>(i), 0, -offset};
            vertices.push_back(vertex);
        }

        // Y direction
        for (int i = 0; i < nb_lines; i++) {
            vertex.position = {offset, 0, -offset + line_spacing * static_cast<float>(i)};
            vertices.push_back(vertex);
            vertex.position = {-offset, 0, -offset + line_spacing * static_cast<float>(i)};
            vertices.push_back(vertex);
        }

        for (int i = 0; i < nb_lines * 4; i++) {
            elements.push_back(i);
        }

        // todo: Create default resources for each type so we can do world.get_default<rsc::Material>().
        //       This will prevent having too much resources.
        const auto mesh = world.create_resource<rsc::Mesh>(
                "grid_mesh", world.create_resource<rsc::Material>("material"), vertices, elements);

        mesh->set_gl_primitive_type(GL_LINES);
        mesh->update();

        mesh->get_material()->do_lighting(false);

        // Create model
        std::shared_ptr<rsc::Model> model = world.create_resource<rsc::Model>("grid_model", mesh);


        // Create entity
        const EntityID entity = world.create_entity("Grid");
        world.attach_component<cpt::StaticModel>(entity, model);
        world.attach_component<cpt::Transform>(entity);

        return entity;
    }

    EntityID create_axis(World &world, float axis_length) {
        // Create the mesh
        // todo: make it not lighted

        std::vector<rsc::Vertex> vertices;
        std::vector<rsc::VertexID> elements;
        vertices.reserve(6); // 3 lines so 6 vertices
        elements.reserve(6);

        // X => R
        vertices.push_back(rsc::Vertex({0, 0, 0}, {1, 1, 1}, {0, 0}, {1, 0, 0}));
        vertices.push_back(rsc::Vertex({axis_length, 0, 0}, {1, 1, 1}, {0, 0}, {1, 0, 0}));

        // Y => G
        vertices.push_back(rsc::Vertex({0, 0, 0}, {1, 1, 1}, {0, 0}, {0, 1, 0}));
        vertices.push_back(rsc::Vertex({0, axis_length, 0}, {1, 1, 1}, {0, 0}, {0, 1, 0}));

        // Z => B
        vertices.push_back(rsc::Vertex({0, 0, 0}, {1, 1, 1}, {0, 0}, {0, 0, 1}));
        vertices.push_back(rsc::Vertex({0, 0, axis_length}, {1, 1, 1}, {0, 0}, {0, 0, 1}));

        for (int i = 0; i < 6; i++) {
            elements.push_back(i);
        }

        const auto mesh = world.create_resource<rsc::Mesh>(
                "axis_mesh", world.create_resource<rsc::Material>("material"), vertices, elements);
        mesh->set_gl_primitive_type(GL_LINES);
        mesh->update();

        mesh->get_material()->do_lighting(false);

        // Create model
        std::shared_ptr<rsc::Model> model = world.create_resource<rsc::Model>("axis_model", mesh);

        // Create entity
        const EntityID entity = world.create_entity("Axis");
        world.attach_component<cpt::StaticModel>(entity, model);
        world.attach_component<cpt::Transform>(entity);

        return entity;
    }


} // namespace wrld::builtins
