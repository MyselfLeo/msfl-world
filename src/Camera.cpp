//
// Created by leo on 8/10/25.
//

#include "Camera.hpp"

#include "GLFW/glfw3.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"

namespace wrld {
    Camera::Camera(GLFWwindow *window) :
        window(window), position(glm::vec3(1.0f)), rotation(glm::quat{1.0f, 0.0f, 0.0f, 0.0f}), fov(45.0f) {}

    glm::vec3 Camera::get_position() const { return this->position; }

    glm::quat Camera::get_rotation() const { return this->rotation; }

    float Camera::get_fov() const { return this->fov; }

    void Camera::set_position(const glm::vec3 &position) { this->position = position; }

    void Camera::set_rotation(const glm::quat &rotation) { this->rotation = rotation; }

    void Camera::set_rotation(const glm::vec3 &rotation) {
        this->rotation = glm::quat(glm::vec3(rotation.x, rotation.y, rotation.z));
    }

    void Camera::set_fov(const float fov) { this->fov = fov; }

    glm::mat4x4 Camera::view_matrix() const { return glm::translate(glm::toMat4(this->rotation), this->position); }

    glm::mat4x4 Camera::projection_matrix() const {
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        const float ratio = static_cast<float>(width) / static_cast<float>(height);
        return glm::perspective(glm::radians(this->fov), ratio, 0.1f, 100.0f);
    }
} // namespace wrld
