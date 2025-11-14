//
// Created by leo on 9/23/25.
//

#include <wrld/World.hpp>
#include <wrld/tools/ModelTool.hpp>

namespace wrld::tools {
    std::vector<Rc<rsc::Model>>
    ModelTool::split_in_grid(World &world, const Rc<rsc::Model> &source_model,
                             const float grid_size) {
        // Define the grid.
        // For simplification the grid will be aligned with the bounding box of
        // source_model.
        const obj::Box source_bb = source_model->get_bounding_box();

        const glm::vec3 source_bb_size = source_bb.size();

        // Number of boxes on each coordinates
        const unsigned x_count = std::ceil(source_bb_size.x / grid_size);
        const unsigned y_count = std::ceil(source_bb_size.y / grid_size);
        const unsigned z_count = std::ceil(source_bb_size.z / grid_size);

        const unsigned max_mesh_count = x_count * y_count * z_count;

        // Compute the meshgroups & materials to set in each model
        std::vector<std::vector<obj::MeshGroup>> mesh_groups(max_mesh_count);
        std::vector<std::vector<Rc<rsc::Material>>> materials(max_mesh_count);

        const auto src_meshgroups = source_model->get_mesh_groups();
        const auto src_materials = source_model->get_materials();

        for (const auto &[mg, mat]: src_meshgroups) {
            // Split the mesh group
            const auto splitted_mg = split_mesh_group(mg, grid_size, source_bb);
            for (int i = 0; i < splitted_mg.size(); i++) {
                if (splitted_mg[i].get_element_count() == 0)
                    continue;

                mesh_groups[i].push_back(splitted_mg[i]);
                materials[i].push_back(src_materials[mat]);
            }
        }

        // Create the new models
        std::vector<Rc<rsc::Model>> new_models;
        new_models.reserve(max_mesh_count);

        for (int i = 0; i < max_mesh_count; i++) {
            // Don't create empty models
            if (mesh_groups[i].empty())
                continue;

            std::string new_name =
                    std::format("{}_split_{}", source_model->get_name(), i);

            const auto new_model = world.create_resource<rsc::Model>(new_name);

            new_model->from_mesh_groups(mesh_groups[i], materials[i]);
            new_models.push_back(new_model);
        }

        return new_models;
    }

    std::vector<obj::Mesh> ModelTool::split_mesh(const obj::Mesh &mesh,
                                                 const float grid_size,
                                                 const obj::Box &global_box) {
        const auto offset = global_box.lower();

        // Number of boxes on each coordinates
        const unsigned x_count = std::ceil(global_box.size().x / grid_size);
        const unsigned y_count = std::ceil(global_box.size().y / grid_size);
        const unsigned z_count = std::ceil(global_box.size().z / grid_size);
        const unsigned max_mesh_count = x_count * y_count * z_count;

        // Prepare the new meshes
        std::vector<obj::Mesh> new_meshes(max_mesh_count);
        for (auto &m: new_meshes) {
            m.set_primitive_type(mesh.get_primitive_type());
        }

        const auto &source_vertices = mesh.get_vertices();
        const auto &source_elements = mesh.get_elements();

        // For each newly created mesh, we store the vertices already added.
        // It prevents duplication in the mesh vertex list.
        std::vector<std::unordered_map<obj::VertexID, obj::VertexID>> assigned_vertices(
                max_mesh_count);

        /// Add a new element index to the specified mesh.
        /// Will add the corresponding vertex in the mesh's vertices if required.
        const auto add_mesh_element = [&](const obj::VertexID vertex_id,
                                          const unsigned mesh_id) -> void {
            if (!assigned_vertices[mesh_id].contains(vertex_id)) {
                const auto &vertex = source_vertices[vertex_id];
                assigned_vertices[mesh_id].insert_or_assign(
                        vertex_id, new_meshes[mesh_id].add_vertex(vertex));
            }
            const obj::VertexID id = assigned_vertices[mesh_id][vertex_id];
            new_meshes[mesh_id].add_element(id);
        };

        // Iterate over each element of the mesh.
        const unsigned prim_size = obj::get_primitive_size(mesh.get_primitive_type());
        for (unsigned prim_id = 0; prim_id < source_elements.size();
             prim_id += prim_size) {

            // Compute center of the primitive
            glm::vec3 center{0};
            for (unsigned e_id = 0; e_id < prim_size; e_id++) {
                center += source_vertices[source_elements[prim_id + e_id]].position;
            }
            center = glm::vec3{center.x / prim_size, center.y / prim_size,
                               center.z / prim_size};

            const glm::vec3 offset_center = center - offset;

            const unsigned i_x = std::floor(offset_center.x / grid_size);
            const unsigned i_y = std::floor(offset_center.y / grid_size);
            const unsigned i_z = std::floor(offset_center.z / grid_size);
            const unsigned mesh_id = i_x + i_y * x_count + i_z * (x_count * y_count);

            for (unsigned e_id = 0; e_id < prim_size; e_id++) {
                add_mesh_element(source_elements[prim_id + e_id], mesh_id);
            }
        }

        return new_meshes;
    }

    std::vector<obj::MeshGroup>
    ModelTool::split_mesh_group(const obj::MeshGroup &mesh_group, const float grid_size,
                                const obj::Box &global_box) {
        // Number of boxes on each coordinates
        const unsigned x_count = std::ceil(global_box.size().x / grid_size);
        const unsigned y_count = std::ceil(global_box.size().y / grid_size);
        const unsigned z_count = std::ceil(global_box.size().z / grid_size);
        const unsigned max_mesh_count = x_count * y_count * z_count;

        std::vector<obj::MeshGroup> new_groups(max_mesh_count);

        for (const auto &m: mesh_group.get_meshes()) {
            const auto &split_meshes = split_mesh(m, grid_size, global_box);

            for (const auto &[sm_id, sm]: split_meshes | std::views::enumerate) {
                new_groups[sm_id].add_mesh(sm);
            }
        }

        return new_groups;
    }
} // namespace wrld::tools
