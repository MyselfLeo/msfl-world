//
// Created by leo on 8/11/25.
//

#include "TransformComponent.hpp"

#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/transform.hpp"

namespace wrld {
    TransformComponent::TransformComponent(const EntityID entity_id, World &world, const glm::vec3 &position,
                                           const glm::quat &rotation, const glm::vec3 &scale) :
        Component(entity_id, world), position(position), rotation(rotation), scale(scale) {}

    glm::vec3 TransformComponent::get_position() const { return position; }

    glm::quat TransformComponent::get_rotation() const { return rotation; }

    glm::vec3 TransformComponent::get_scale() const { return scale; }

    void TransformComponent::set_position(const glm::vec3 &position) { this->position = position; }

    void TransformComponent::set_rotation(const glm::quat &rotation) { this->rotation = rotation; }

    void TransformComponent::set_scale(const glm::vec3 &scale) { this->scale = scale; }

    glm::mat4x4 TransformComponent::model_matrix() const {
        return glm::translate(this->position) * glm::toMat4(this->rotation) * glm::scale(this->scale);
    }
} // namespace wrld
