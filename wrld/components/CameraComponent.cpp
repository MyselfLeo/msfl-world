//
// Created by leo on 8/13/25.
//

#include "glm/gtc/matrix_transform.hpp"

#include "CameraComponent.hpp"

#include "TransformComponent.hpp"
#include "World.hpp"

#include <iostream>

namespace wrld {
    CameraComponent::CameraComponent(const EntityID entity_id, World &world, const float fov) :
        Component(entity_id, world), fov(fov) {}

    float CameraComponent::get_fov() const { return fov; }

    void CameraComponent::set_fov(const float fov) { this->fov = fov; }

    glm::mat4x4 CameraComponent::get_view_matrix() const {
        if (const auto transform_cmpnt = world.get_component<TransformComponent>(entity_id)) {
            return transform_cmpnt.value()->model_matrix();
        }
        return glm::mat4x4(1.0);
    }

    glm::mat4x4 CameraComponent::get_projection_matrix(const unsigned width, const unsigned height) const {
        const float ratio = static_cast<float>(width) / static_cast<float>(height);
        return glm::perspective(glm::radians(this->fov), ratio, 0.1f, 100.0f);
    }
} // namespace wrld
