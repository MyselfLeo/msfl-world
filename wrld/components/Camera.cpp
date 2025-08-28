//
// Created by leo on 8/13/25.
//

#include "glm/gtc/matrix_transform.hpp"

#include "Camera.hpp"

#include "Logs.hpp"
#include "Transform.hpp"
#include "World.hpp"

#include <iostream>

namespace wrld::cpt {
    const glm::vec3 Camera::UP_VECTOR = glm::vec3(0, 1, 0);

    Camera::Camera(const EntityID entity_id, World &world, const float fov, std::shared_ptr<Program> program) :
        Component(entity_id, world), fov(fov), program(std::move(program)) {}

    std::shared_ptr<Program> Camera::get_program() const { return program; }

    void Camera::set_program(const std::shared_ptr<Program> &program) { this->program = program; }

    float Camera::get_fov() const { return fov; }

    void Camera::set_fov(const float fov) { this->fov = fov; }

    glm::mat4x4 Camera::get_view_matrix() const {
        if (const auto transform_cmpnt = world.get_component_opt<Transform>(entity_id)) {
            return glm::inverse(transform_cmpnt.value()->model_matrix());
        }
        return glm::mat4x4(1.0);
    }

    glm::mat4x4 Camera::get_projection_matrix(const unsigned width, const unsigned height) const {
        const float ratio = static_cast<float>(width) / static_cast<float>(height);
        return glm::perspective(glm::radians(this->fov), ratio, 0.1f, 1000.0f);
    }

    glm::mat4x4 Camera::get_viewport_matrix(const unsigned width, const unsigned height) {
        const float w = width / 2.f;
        const float h = height / 2.f;

        return glm::mat4x4{w, 0, 0, 0, 0, h, 0, 0, 0, 0, 0.5, 0, w, h, 0.5, 1};
    }

    glm::vec3 Camera::get_position() const {
        if (const auto transform_cmpnt = world.get_component_opt<Transform>(entity_id)) {
            return transform_cmpnt.value()->get_position();
        }
        return glm::vec3(0.0);
    }
} // namespace wrld::cpt
