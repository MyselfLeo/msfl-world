//
// Created by leo on 8/11/25.
//

#include <wrld/components/Transform.hpp>

#include "glm/gtx/quaternion.hpp"
#include "glm/gtx/transform.hpp"

namespace wrld::cpt {
    Transform::Transform(const EntityID entity_id, World &world, const glm::vec3 &position, const glm::quat &rotation,
                         const glm::vec3 &scale) :
        Component(entity_id, world), position(position), rotation(rotation), scale(scale) {}

    glm::vec3 Transform::get_position() const { return position; }

    glm::quat Transform::get_rotation() const { return rotation; }

    glm::vec3 Transform::get_scale() const { return scale; }

    glm::vec3 Transform::get_direction() const { return rotation_matrix() * glm::vec4{0, 0, -1, 0}; }

    void Transform::set_position(const glm::vec3 &position) { this->position = position; }

    void Transform::set_rotation(const glm::quat &rotation) { this->rotation = glm::normalize(rotation); }

    void Transform::set_scale(const glm::vec3 &scale) { this->scale = scale; }

    void Transform::look_at(const glm::vec3 &target, const glm::vec3 &up) {
        rotation = glm::quat(glm::inverse(glm::lookAt(position, target, up)));
    }

    void Transform::look_towards(const glm::vec3 &direction, const glm::vec3 &up) {
        look_at(position + glm::normalize(direction), up);
    }

    glm::mat4x4 Transform::model_matrix() const { return translate_matrix() * rotation_matrix() * scale_matrix(); }

    glm::mat4x4 Transform::translate_matrix() const { return glm::translate(this->position); }

    glm::mat4x4 Transform::rotation_matrix() const { return glm::toMat4(this->rotation); }

    glm::mat4x4 Transform::scale_matrix() const { return glm::scale(this->scale); }
} // namespace wrld::cpt
