//
// Created by leo on 9/6/25.
//

#pragma once

#include <vector>
#include <glad/glad.h>
#include <wrld/objects/geometry/AABoundingBox.hpp>

namespace wrld::obj {
    enum class PrimitiveType {
        Points,
        Lines,
        Triangles,
    };

    /// Return the size of a primitive (point -> 1, line -> 2, triangle -> 3...).
    unsigned get_primitive_size(PrimitiveType type);

    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texcoords;
        glm::vec3 color;
    };

    typedef GLuint VertexID;
    typedef GLuint ElementID;

    class Mesh {
    public:
        /// Create an empty mesh with a given primitive type.
        explicit Mesh(PrimitiveType primitive_type = PrimitiveType::Triangles);

        /// Set the set of vertices of this mesh.
        /// Be sure to update the elements (Mesh::set_elements) if necessary.
        Mesh &set_vertices(const std::vector<Vertex> &vertices);

        //// Set the set of elements of this mesh.
        /// In the case of PrimitiveType::Triangles, every 3 VertexID represents
        /// a triangle. In the case of PrimitiveType::Lines, it's every 2, etc.
        Mesh &set_elements(const std::vector<VertexID> &elements);

        /// Add a vertex to the mesh.
        VertexID add_vertex(const Vertex &vertex);

        /// Return a specific vertex of the mesh.
        Vertex &get_vertex(VertexID vertex_id);

        /// Return the vertices of the mesh.
        const std::vector<Vertex> &get_vertices() const;

        /// Return the number of vertices.
        [[nodiscard]] unsigned get_vertex_count() const;

        /// Add an element index to the mesh.
        ElementID add_element(VertexID vertex_id);

        /// Return a specific element index of the mesh.
        VertexID &get_element(ElementID element_id);

        /// Return the element indices of the mesh.
        [[nodiscard]] const std::vector<VertexID> &get_elements() const;

        /// Return the number of element indices.
        [[nodiscard]] unsigned get_element_count() const;

        /// Return the primitive type of this mesh.
        [[nodiscard]] PrimitiveType get_primitive_type() const;

        /// Set the primitive type of the mesh. Be sure to
        /// update vertices & elements if necessary.
        void set_primitive_type(PrimitiveType primitive_type);

        /// Recompute the normals of the mesh based on triangle orientation.
        /// An average of the normals of every triangle around a vertex is
        /// computed and is set to be this vertex's normal.
        /// Throws an error if self is not PrimitiveType::triangle.
        void recompute_normals();

        /// Return the axis-aligned bounding-box of the mesh.
        [[nodiscard]] AABoundingBox get_bounding_box() const;

        /// Clears mesh data (vertices & elements) of this mesh.
        /// PrimitiveType and bounding_box are not modified.
        void clear();

    private:
        std::vector<Vertex> vertices;
        std::vector<VertexID> indices;
        PrimitiveType primitive_type;

        AABoundingBox bounding_box;
    };
} // namespace wrld::obj
