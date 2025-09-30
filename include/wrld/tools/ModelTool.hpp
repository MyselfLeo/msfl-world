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
        static std::vector<Rc<rsc::Model>> split_in_grid(World &world, const Rc<rsc::Model> &source_model,
                                                         float grid_size);
    };

} // namespace wrld::tools
