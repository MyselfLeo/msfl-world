//
// Created by leo on 9/6/25.
//

#include "Mesh.hpp"

namespace wrld::rsc {
    Mesh::Mesh(const std::shared_ptr<Material> &default_material) :
        default_material(default_material), current_material(default_material) {}

    Mesh::Mesh(const std::shared_ptr<Material> &default_material, const std::vector<Vertex> &vertices,
               const std::vector<VertexID> &elements) :
        vertices(vertices), indices(elements), default_material(default_material), current_material(default_material) {}

    Mesh::Mesh(Mesh &&other) noexcept :
        vertices(std::move(other.vertices)), indices(std::move(other.indices)),
        default_material(std::move(other.default_material)), current_material(std::move(other.current_material)),
        gl_primitive_type(other.gl_primitive_type), vao(other.vao), vbo(other.vbo), ebo(other.ebo) {
        other.vao = 0;
        other.vbo = 0;
        other.ebo = 0;
    }

    Mesh &Mesh::operator=(Mesh &&other) noexcept {
        if (this != &other) {
            if (vao != 0)
                glDeleteVertexArrays(1, &vao);
            if (vbo != 0)
                glDeleteBuffers(1, &vbo);
            if (ebo != 0)
                glDeleteBuffers(1, &ebo);

            // Transfer resources
            vertices = std::move(other.vertices);
            indices = std::move(other.indices);
            // textures = std::move(other.textures);
            vao = other.vao;
            vbo = other.vbo;
            ebo = other.ebo;
            gl_primitive_type = other.gl_primitive_type;

            // Reset source object
            other.vao = 0;
            other.vbo = 0;
            other.ebo = 0;
        }
        return *this;
    }

    Mesh::~Mesh() {
        glBindVertexArray(0);
        glDeleteBuffers(1, &ebo);
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
    }

    void Mesh::set_material(const std::shared_ptr<Material> &material) { this->current_material = material; }

    void Mesh::use_default_material() { this->current_material = default_material; }

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

    void Mesh::set_gl_primitive_type(const GLenum type) { this->gl_primitive_type = type; }

    GLenum Mesh::get_gl_primitive_type() const { return gl_primitive_type; }

    void Mesh::set_gl_usage(const GLenum usage) { this->gl_usage = usage; }

    GLenum Mesh::get_gl_usage() const { return gl_usage; }

    const std::shared_ptr<Material> &Mesh::get_material() const { return current_material; }

    void Mesh::update() {
        if (!buffers_created) {
            glGenVertexArrays(1, &vao);
            glGenBuffers(1, &vbo);
            glGenBuffers(1, &ebo);
        }
        buffers_created = true;


        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned), indices.data(), GL_STATIC_DRAW);

        // Vertex positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), static_cast<void *>(nullptr));

        // Vertex normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              reinterpret_cast<void *>(offsetof(Vertex, normal)));

        // Vertex colors
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              reinterpret_cast<void *>(offsetof(Vertex, color)));


        // Vertex texture coordinates
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              reinterpret_cast<void *>(offsetof(Vertex, texcoords)));

        glBindVertexArray(0);
    }

    GLuint Mesh::get_vao() const { return vao; }

    unsigned Mesh::get_element_count() const { return indices.size(); }
} // namespace wrld::rsc
