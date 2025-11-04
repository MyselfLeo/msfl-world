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

        std::vector<obj::Box> boxes(x_count * y_count * z_count);

        // Create the bounding boxes
        for (int x = 0; x < x_count; x++) {
            for (int y = 0; y < y_count; y++) {
                for (int z = 0; z < z_count; z++) {
                    const float offset_x = x * grid_size;
                    const float offset_y = y * grid_size;
                    const float offset_z = z * grid_size;

                    glm::vec3 lower =
                            source_bb.lower() + glm::vec3{offset_x, offset_y, offset_z};
                    glm::vec3 upper = lower + glm::vec3{grid_size};

                    boxes[x + y * y_count + z * (x_count + y_count)] =
                            obj::Box::bounding_box(lower, upper);
                }
            }
        }

        // Compute the meshgroups & materials to set in each model
        std::vector<std::vector<obj::MeshGroup>> mesh_groups(boxes.size());
        std::vector<std::vector<Rc<rsc::Material>>> materials(boxes.size());

        const auto src_meshgroups = source_model->get_mesh_groups();
        const auto src_materials = source_model->get_materials();

        for (const auto &[mg, mat]: src_meshgroups) {
            // Split the mesh group
            const auto splitted_mg = split_mesh_group(mg, boxes);
            for (int i = 0; i < boxes.size(); i++) {
                if (splitted_mg[i].get_element_count() == 0)
                    continue;

                mesh_groups[i].push_back(splitted_mg[i]);
                materials[i].push_back(src_materials[mat]);
            }
        }

        // Create the new models
        std::vector<Rc<rsc::Model>> new_models;
        new_models.reserve(boxes.size());

        for (int i = 0; i < boxes.size(); i++) {
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

    std::vector<obj::Mesh>
    ModelTool::split_mesh(const obj::Mesh &mesh,
                          const std::vector<obj::Box> &bounding_boxes) {

        // Prepare the new meshes
        std::vector<obj::Mesh> new_meshes(bounding_boxes.size());
        for (auto &m: new_meshes) {
            m.set_primitive_type(mesh.get_primitive_type());
        }

        const auto &source_vertices = mesh.get_vertices();
        const auto &source_elements = mesh.get_elements();

        // For each newly created mesh, we store the vertices already added.
        // It prevents duplication in the mesh vertex list.
        std::vector<std::unordered_map<obj::VertexID, obj::VertexID>> assigned_vertices(
                bounding_boxes.size());

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

            // We have to choose a bounding box for this primitive. For now, we'll use
            // its first coordinate, but maybe we can compute its center for example ?
            const glm::vec3 center = source_vertices[prim_id].position;

            // Find the bounding box that contains this point
            for (const auto &[b_id, b]: bounding_boxes | std::views::enumerate) {
                if (b.inside(center)) {
                    // Add the elements to the correct mesh
                    for (unsigned e_id = 0; e_id < prim_size; e_id++) {
                        add_mesh_element(prim_id + e_id, b_id);
                    }
                    break;
                }
            }
        }

        return new_meshes;
    }

    std::vector<obj::MeshGroup>
    ModelTool::split_mesh_group(const obj::MeshGroup &mesh_group,
                                const std::vector<obj::Box> &bounding_boxes) {
        std::vector<obj::MeshGroup> new_groups(bounding_boxes.size());

        for (const auto &m: mesh_group.get_meshes()) {
            const auto &split_meshes = split_mesh(m, bounding_boxes);

            for (const auto &[sm_id, sm]: split_meshes | std::views::enumerate) {
                new_groups[sm_id].add_mesh(sm);
            }
        }

        return new_groups;
    }

    // Rc<rsc::Model> ModelTool::generate_bb_model(World &world,
    //                                             const obj::Box &bounding_box) {
    //     std::vector<rsc::Vertex> vertices{};
    //     vertices.reserve(8);
    //
    //     // see BoundingBox.hpp
    //     std::vector<rsc::VertexID> elements = {
    //             0,
    //             1,
    //
    //     };
    //
    //     for (const auto &p: bounding_box.vertices()) {
    //         vertices.push_back({p, glm::vec3{0}, glm::vec3{0}, glm::vec4{1.0}});
    //     }
    //
    //     const Rc<rsc::Mesh> mesh = world.create_resource<rsc::Mesh>("bounding_box");
    //     mesh->set_primitive_type(GL_LINES);
    // }

} // namespace wrld::tools
