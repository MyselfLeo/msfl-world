//
// Created by leo on 8/10/25.
//

#include "Camera.hpp"

#include "glm/gtc/matrix_transform.hpp"

namespace wrld {
    Camera::Camera(const glm::vec3 position, const glm::vec3 rotation) : position(position), rotation(rotation) {}

    glm::mat4x4 Camera::view_matrix() const { return glm::translate(glm::mat4x4(1.0f), this->position); }
    glm::mat4x4 Camera::projection_matrix() const {
        return glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
    }
} // namespace wrld
