//
// Created by leo on 9/6/25.
//

#include <complex>
#include <wrld/resources/Mesh.hpp>

#include <wrld/World.hpp>

#include <utility>

#include "glm/detail/func_geometric.hpp"

namespace wrld::rsc {
    Mesh::Mesh(std::string name, World &world) : Resource(std::move(name), world) {
        attach_resource("current_material", world.get_default<Material>());
    }

    // Mesh::Mesh(std::string name, World &world, const std::shared_ptr<Material> &default_material,
    //            const std::vector<Vertex> &vertices, const std::vector<VertexID> &elements) :
    //     Resource(std::move(name), world), vertices(vertices), indices(elements), default_material(default_material),
    //     current_material(default_material) {}

    Mesh::~Mesh() {
        // glBindVertexArray(0);
        // glDeleteBuffers(1, &ebo);
        // glDeleteBuffers(1, &vbo);
        // glDeleteVertexArrays(1, &vao);
    }

    Mesh &Mesh::set_material(const Rc<Material> &material) {
        attach_resource("current_material", material);
        return *this;
    }

    Mesh &Mesh::set_vertices(const std::vector<Vertex> &vertices) {
        this->vertices = vertices;
        return *this;
    }

    Mesh &Mesh::set_elements(const std::vector<VertexID> &elements) {
        this->indices = elements;
        return *this;
    }

    // void Mesh::set_material(const std::shared_ptr<Material> &material) { this->current_material = material; }
    //
    // void Mesh::use_default_material() { this->current_material = default_material; }

    VertexID Mesh::add_vertex(const Vertex &vertex) {
        this->vertices.push_back(vertex);
        return this->vertices.size() - 1;
    }

    Vertex &Mesh::get_vertex(const VertexID vertex_id) { return this->vertices[vertex_id]; }

    ElementID Mesh::add_element(const VertexID vertex_id) {
        this->indices.push_back(vertex_id);
        return this->indices.size() - 1;
    }

    VertexID &Mesh::get_element(const ElementID element_id) { return this->indices[element_id]; }

    // Mesh &Mesh::set_gl_primitive_type(const GLenum type) {
    //     this->gl_primitive_type = type;
    //     return *this;
    // }
    //
    // GLenum Mesh::get_gl_primitive_type() const { return gl_primitive_type; }

    Mesh &Mesh::set_gl_usage(const GLenum usage) {
        this->gl_usage = usage;
        return *this;
    }

    GLenum Mesh::get_gl_usage() const { return gl_usage; }

    Rc<Material> Mesh::get_material() const { return get_resource<Material>("current_material"); }

    void Mesh::recompute_normals() {
        if (indices.size() % 3 != 0) {
            throw std::runtime_error("Mesh doesn't look triangle");
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

    unsigned Mesh::get_element_count() const { return indices.size(); }
} // namespace wrld::rsc
