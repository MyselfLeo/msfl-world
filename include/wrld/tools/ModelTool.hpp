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

        /// Given a list of bounding boxes and a mesh, split this mesh in each bounding
        /// box.
        /// Note that the resulting meshes won't be totally inside their defined
        /// bounding-box; some triangles will be between 2 bounds and thus will fall in
        /// either one of the 2 boxes. You can get the mesh bounding box using
        /// Mesh::get_bounding_box.
        /// Assumes that each bounding box is independant.
        static std::vector<obj::Mesh>
        split_mesh(const obj::Mesh &mesh, const std::vector<obj::Box> &bounding_boxes);

        /// Same as ModelTool::split_mesh.
        static std::vector<obj::MeshGroup>
        split_mesh_group(const obj::MeshGroup &mesh_group,
                         const std::vector<obj::Box> &bounding_boxes);
    };

} // namespace wrld::tools
