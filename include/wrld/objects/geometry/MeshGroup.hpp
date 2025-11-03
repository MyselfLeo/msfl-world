//
// Created by leo on 10/26/25.
//

#pragma once
#include <wrld/objects/geometry/Mesh.hpp>

namespace wrld::obj {
    struct MeshGroupAggregate {
        std::vector<Vertex> vertices;
        std::vector<VertexID> elements;
        std::vector<int> mesh_starts;
        std::vector<int> mesh_sizes;
    };

    /// A set of meshes sharing the same PrimitiveType.
    class MeshGroup {
    public:
        /// Creates an empty MeshGroup.
        MeshGroup() = default;

        /// Creates a MeshGroup with a given set of meshes.
        /// Fails if every mesh are not of the same PrimitiveType.
        explicit MeshGroup(const std::vector<Mesh> &meshes);

        /// Add a mesh to the group.
        /// Fails if the mesh is not of the same PrimitiveType as the rest.
        void add_mesh(const Mesh &mesh);

        /// Return the number of meshes in the group.
        size_t get_mesh_count() const;

        /// Return the meshes of this group.
        const std::vector<Mesh> &get_meshes() const;

        /// Return the PrimitiveType of the meshes of this group.
        /// Fails if the group is empty.
        PrimitiveType get_primitive_type() const;

        /// Return a structure with all meshes vertices stored in a single
        /// vector, alongside the start & size of each mesh.
        /// This function is quite compute-heavy so don't call too often.
        MeshGroupAggregate get_aggregate() const;

        /// Return the number of vertices.
        [[nodiscard]] unsigned get_vertex_count() const;

        /// Return the number of element indices.
        [[nodiscard]] unsigned get_element_count() const;

    private:
        std::vector<Mesh> meshes;
    };

} // namespace wrld::obj
