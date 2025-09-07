//
// Created by leo on 8/13/25.
//

#include "StaticModel.hpp"

namespace wrld::cpt {
    StaticModel::StaticModel(const EntityID entity_id, World &world, const std::shared_ptr<rsc::Model> &model) :
        Component(entity_id, world), model(model) {}

    const std::shared_ptr<rsc::Model> &StaticModel::get_model() const { return model; }

    void StaticModel::set_model(const std::shared_ptr<rsc::Model> &model) { this->model = model; }
} // namespace wrld::cpt
