//
// Created by leo on 10/26/25.
//

#include <stdexcept>
#include <wrld/objects/geometry/MeshGroup.hpp>

namespace wrld::obj {
    MeshGroup::MeshGroup(const std::vector<Mesh> &meshes) {
        if (meshes.empty())
            return;

        // Type check
        const PrimitiveType type = meshes[0].get_primitive_type();
        for (const auto &m: meshes) {
            if (m.get_primitive_type() != type) {
                throw std::runtime_error("All meshes are not of the same type.");
            }
        }

        this->meshes = meshes;
    }

    void MeshGroup::add_mesh(const Mesh &mesh) {
        if (!meshes.empty() && mesh.get_primitive_type() != get_primitive_type()) {
            throw std::runtime_error("The mesh is not of the same type as the group.");
        }

        meshes.push_back(mesh);
    }

    size_t MeshGroup::get_mesh_count() const { return meshes.size(); }

    const std::vector<Mesh> &MeshGroup::get_meshes() const { return meshes; }

    PrimitiveType MeshGroup::get_primitive_type() const {
        if (meshes.empty()) {
            throw std::runtime_error("The mesh is empty.");
        }

        return meshes[0].get_primitive_type();
    }

    MeshGroupAggregate MeshGroup::get_aggregate() const {
        MeshGroupAggregate res;
        res.mesh_starts.reserve(get_mesh_count());
        res.mesh_sizes.reserve(get_mesh_count());

        size_t nb_vertices = 0;
        size_t nb_elements = 0;
        for (const auto &m: meshes) {
            nb_vertices += m.get_vertex_count();
            nb_elements += m.get_element_count();
        }

        res.vertices.reserve(nb_vertices);
        res.elements.reserve(nb_elements);

        // I wanted to de-dupe the vertices but I'm not sure it's really useful.
        // How many meshes will share the same Vertex (= same position, same
        // normal, same texcoord, same color) ?

        // Fill the vectors
        for (const auto &m: meshes) {
            res.mesh_starts.push_back(res.elements.size());
            res.mesh_sizes.push_back(m.get_element_count());

            for (const auto &e: m.get_elements()) {
                res.elements.push_back(e + res.vertices.size());
            }

            res.vertices.insert(res.vertices.end(), m.get_vertices().begin(),
                                m.get_vertices().end());
        }

        return res;
    }

    unsigned MeshGroup::get_vertex_count() const {
        unsigned s = 0;
        for (const auto &m: meshes) {
            s += m.get_vertex_count();
        }
        return s;
    }

    unsigned MeshGroup::get_element_count() const {
        unsigned s = 0;
        for (const auto &m: meshes) {
            s += m.get_element_count();
        }
        return s;
    }
} // namespace wrld::obj
