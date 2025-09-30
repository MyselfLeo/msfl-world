//
// Created by leo on 9/23/25.
//

#include <wrld/World.hpp>
#include <wrld/tools/ModelTool.hpp>

namespace wrld::tools {
    std::vector<Rc<rsc::Model>> ModelTool::split_in_grid(World &world, const Rc<rsc::Model> &source_model,
                                                         const float grid_size) {
        // Define the grid.
        // For simplification the grid will be aligned with the bounding box of source_model.
        const rsc::BoundingBox source_bb = source_model->get_local_bb();
        const glm::vec3 source_bb_size = source_bb.size();

        if (source_bb_size.x < 0 || source_bb_size.y < 0 || source_bb_size.z < 0) {
            throw std::runtime_error("Source bb cannot be of negative size");
        }

        // Todo: see TODO.md
        // assert(source_model->get_meshes().size() == 1);

        // todo: support multiple materials
        // Rc<rsc::Material> mat = world.get_default<rsc::Material>();
        Rc<rsc::Material> mat = source_model->get_materials().at(0);

        const unsigned x_count = std::ceil(source_bb_size.x / grid_size);
        const unsigned y_count = std::ceil(source_bb_size.y / grid_size);
        const unsigned z_count = std::ceil(source_bb_size.z / grid_size);

        // Create a single mesh for each model
        // todo: support multiple materials. We'll need to create 1 mesh for each material for each model
        std::vector<Rc<rsc::Mesh>> new_meshes;
        const unsigned new_mesh_count = x_count * y_count * z_count;
        new_meshes.reserve(new_mesh_count);

        // /// Return (x, y, z) coordinates based on the given index.
        // /// Expects that the vector is indexed using firstly x, then y, then z.
        // const auto get_coo = [&](const unsigned pos) -> std::tuple<unsigned, unsigned, unsigned> {
        //     unsigned z = pos % z_count;
        //     unsigned y = pos % (y_count * z_count) / z_count;
        //     unsigned x = pos / (y_count * z_count);
        //
        //     return {x, y, z};
        // };

        const auto &source_elements = source_model->get_elements();
        const auto &source_vertices = source_model->get_vertices();

        // For each newly created meshes, we store the vertices already added.
        std::vector<std::unordered_map<rsc::VertexID, rsc::VertexID>> assigned_vertices(new_mesh_count);

        /// Add a new element to the mesh.
        /// Prevents repeating vertices in the VBO of the mesh, thanks to assigned_vertices.
        const auto add_mesh_element = [&](const rsc::VertexID vertex_id, const unsigned mesh_id) -> void {
            if (!assigned_vertices[mesh_id].contains(vertex_id)) {
                const auto &vertex = source_vertices[vertex_id];
                assigned_vertices[mesh_id].insert_or_assign(vertex_id, new_meshes[mesh_id]->add_vertex(vertex));
            }
            const rsc::VertexID id = assigned_vertices[mesh_id].at(vertex_id);
            new_meshes.at(mesh_id)->add_element(id);
        };

        for (int x = 0; x < x_count; x++) {
            for (int y = 0; y < y_count; y++) {
                for (int z = 0; z < z_count; z++) {
                    new_meshes.push_back(world.create_resource<rsc::Mesh>(
                            std::format("{}_{}-{}-{}", source_model->get_name(), x, y, z)));
                    new_meshes.back()->set_gl_usage(source_model->get_meshes()[0]->get_gl_usage());
                    new_meshes.back()->set_material(mat);
                }
            }
        }

        // Each triangle of the original model is put in a new_mesh
        for (unsigned triangle_id = 0; triangle_id < source_elements.size(); triangle_id += 3) {
            unsigned e0 = source_elements[triangle_id];
            unsigned e1 = source_elements[triangle_id + 1];
            unsigned e2 = source_elements[triangle_id + 2];

            // Find the mesh in which this triangle will fall. For that, we just use e0's coordinate.
            // todo: It's not the best solution, maybe we can compute the barycentric point ?

            const rsc::Vertex v0 = source_vertices[e0];

            // we add source_bb.lower to shift the coordinates in the positive space
            // we don't change the actual coordinates, but it prevents dealing with negative
            // coordinates
            int mesh_x = std::floor((v0.position.x - source_bb.lower.x) / grid_size);
            int mesh_y = std::floor((v0.position.y - source_bb.lower.y) / grid_size);
            int mesh_z = std::floor((v0.position.z - source_bb.lower.z) / grid_size);

            assert(mesh_x >= 0);
            assert(mesh_y >= 0);
            assert(mesh_z >= 0);

            unsigned target_mesh = mesh_x * (y_count * z_count) + mesh_y * z_count + mesh_z;

            // Add the elements, and thus the vertices, to the mesh.
            add_mesh_element(e0, target_mesh);
            add_mesh_element(e1, target_mesh);
            add_mesh_element(e2, target_mesh);
        }


        // Create the new models, skipping empty meshes if not necessary
        // todo: delete those meshes (see TODO.md)
        std::vector<Rc<rsc::Model>> new_models;

        for (const auto &nm: new_meshes) {
            if (nm->get_element_count() == 0)
                continue;
            const auto &new_model = world.create_resource<rsc::Model>(source_model->get_name());
            new_model->from_mesh(nm);
            new_models.push_back(new_model);
        }

        return new_models;
    }

} // namespace wrld::tools
