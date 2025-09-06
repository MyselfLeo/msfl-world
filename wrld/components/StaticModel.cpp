//
// Created by leo on 8/13/25.
//

#include "StaticModel.hpp"

namespace wrld::cpt {
    StaticModel::StaticModel(const EntityID entity_id, World &world, const rsc::Model &model) :
        Component(entity_id, world), model(model) {}

    const rsc::Model &StaticModel::get_model() const { return model; }

    void StaticModel::set_model(const rsc::Model &model) { this->model = model; }
} // namespace wrld::cpt
