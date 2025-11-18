//
// Created by leo on 9/23/25.
//

#pragma once

#include <wrld/resources/Rc.hpp>
#include <wrld/resources/Model.hpp>


#include <vector>

namespace wrld::tools {
    /// Static class for manipulating Models.
    class ModelTool {
    public:
        /// Creates new models from diving source_model in a 3D grid of grid_size.
        /// The new models will not be centered around (0, 0, 0) in local space : with the
        /// same world transformation, the models will look "attached".
        static std::vector<Rc<rsc::Model>>
        split_in_grid(World &world, const Rc<rsc::Model> &source_model, float grid_size);

    private:
        struct GridData {
            glm::uvec3 grid_dimensions;
            unsigned cell_count = 0;
            float cell_size = 0.0;
            glm::vec3 grid_origin;
        };

        static std::vector<obj::Mesh> split_mesh(const obj::Mesh &mesh,
                                                 const GridData &grid_data);

        /// Same as ModelTool::split_mesh.
        static std::vector<obj::MeshGroup>
        split_mesh_group(const obj::MeshGroup &mesh_group, const GridData &grid_data);
    };

} // namespace wrld::tools
