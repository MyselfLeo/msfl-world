//
// Created by leo on 9/6/25.
//

#include <complex>
#include <../../../include/wrld/objects/geometry/Mesh.hpp>

#include <wrld/World.hpp>

#include <utility>

#include "glm/detail/func_geometric.hpp"

namespace wrld::obj {
    unsigned get_primitive_size(PrimitiveType type) {
        switch (type) {
            case PrimitiveType::Points:
                return 1;
            case PrimitiveType::Lines:
                return 2;
            case PrimitiveType::Triangles:
                return 3;
            default:
                std::unreachable();
        }
    }

    Mesh::Mesh(const PrimitiveType primitive_type) : primitive_type(primitive_type) {}

    Mesh &Mesh::set_vertices(const std::vector<Vertex> &vertices) {
        this->vertices = vertices;
        return *this;
    }

    Mesh &Mesh::set_elements(const std::vector<VertexID> &elements) {
        this->indices = elements;

        // Update bounding box
        bounding_box = AABoundingBox{};
        for (const auto &i: elements) {
            bounding_box.add_point(vertices[i].position);
        }
        return *this;
    }

    VertexID Mesh::add_vertex(const Vertex &vertex) {
        this->vertices.push_back(vertex);
        return this->vertices.size() - 1;
    }

    Vertex &Mesh::get_vertex(const VertexID vertex_id) {
        return this->vertices[vertex_id];
    }

    const std::vector<Vertex> &Mesh::get_vertices() const { return vertices; }

    unsigned Mesh::get_vertex_count() const { return vertices.size(); }

    const std::vector<VertexID> &Mesh::get_elements() const { return indices; }

    unsigned Mesh::get_element_count() const { return indices.size(); }

    ElementID Mesh::add_element(const VertexID vertex_id) {
        this->indices.push_back(vertex_id);

        bounding_box.add_point(vertices[vertex_id].position);

        return this->indices.size() - 1;
    }

    VertexID &Mesh::get_element(const ElementID element_id) {
        return this->indices[element_id];
    }

    PrimitiveType Mesh::get_primitive_type() const { return primitive_type; }

    void Mesh::set_primitive_type(const PrimitiveType primitive_type) {
        this->primitive_type = primitive_type;
    }

    void Mesh::recompute_normals() {
        if (primitive_type != PrimitiveType::Triangles) {
            throw std::runtime_error(
                    "Cannot re-compute normals of a non-triangular mesh");
        }

        std::vector normals(vertices.size(), glm::vec3{0, 0, 0});

        for (int i = 0; i < indices.size(); i += 3) {
            // Compute triangle normal
            const auto &a = vertices[indices[i + 0]].position;
            const auto &b = vertices[indices[i + 1]].position;
            const auto &c = vertices[indices[i + 2]].position;
            const auto ab = b - a;
            const auto ac = c - a;
            const auto normal = glm::cross(ab, ac);

            normals[indices[i + 0]] += normal;
            normals[indices[i + 1]] += normal;
            normals[indices[i + 2]] += normal;
        }

        for (int i = 0; i < vertices.size(); i++) {
            vertices[i].normal = glm::normalize(normals[i]);
        }
    }

    AABoundingBox Mesh::get_bounding_box() const { return bounding_box; }
} // namespace wrld::obj
