//
// Created by leo on 8/13/25.
//

#include "StaticModel.hpp"

namespace wrld::cpt {
    StaticModel::StaticModel(const EntityID entity_id, World &world, const Model &model) :
        Component(entity_id, world), model(model) {}

    const Model &StaticModel::get_model() const { return model; }

    void StaticModel::set_model(const Model &model) { this->model = model; }
} // namespace wrld::cpt
