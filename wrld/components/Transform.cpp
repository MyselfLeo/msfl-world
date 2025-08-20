//
// Created by leo on 8/11/25.
//

#include "Transform.hpp"

#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/transform.hpp"

namespace wrld::cpt {
    Transform::Transform(const EntityID entity_id, World &world, const glm::vec3 &position, const glm::quat &rotation,
                         const glm::vec3 &scale) :
        Component(entity_id, world), position(position), rotation(rotation), scale(scale) {}

    glm::vec3 Transform::get_position() const { return position; }

    glm::quat Transform::get_rotation() const { return rotation; }

    glm::vec3 Transform::get_scale() const { return scale; }

    void Transform::set_position(const glm::vec3 &position) { this->position = position; }

    void Transform::set_rotation(const glm::quat &rotation) { this->rotation = rotation; }

    void Transform::set_scale(const glm::vec3 &scale) { this->scale = scale; }

    glm::mat4x4 Transform::model_matrix() const {
        return glm::translate(this->position) * glm::toMat4(this->rotation) * glm::scale(this->scale);
    }
} // namespace wrld::cpt
