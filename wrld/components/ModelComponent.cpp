//
// Created by leo on 8/13/25.
//

#include "ModelComponent.hpp"

namespace wrld {
    ModelComponent::ModelComponent(const EntityID entity_id, World &world, const Model &model) :
        Component(entity_id, world), model(model) {}

    const Model &ModelComponent::get_model() const { return model; }

    void ModelComponent::set_model(const Model &model) { this->model = model; }
} // namespace wrld
