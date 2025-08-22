//
// Created by leo on 8/20/25.
//

#include "FPSControl.hpp"

#include "Transform.hpp"
#include "World.hpp"
#include "GLFW/glfw3.h"

#include <iostream>

namespace wrld::cpt {
    FPSControl::FPSControl(const EntityID entity_id, World &world) : Component(entity_id, world) {}

    void FPSControl::update(GLFWwindow *window) {
        // Get the entity's transform first.
        // We don't have to do anything if it no longer has one
        const auto transform_opt = world.get_component_opt<Transform>(entity_id);
        if (!transform_opt.has_value())
            return;
        const auto transform = transform_opt.value();
        auto translation = glm::vec3{0.0};

        constexpr auto up = glm::vec3{0, 1, 0};

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            translation += transform->get_direction() * translation_speed;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            translation -= transform->get_direction() * translation_speed;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            translation -= glm::normalize(glm::cross(transform->get_direction(), up)) * translation_speed;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            translation += glm::normalize(glm::cross(transform->get_direction(), up)) * translation_speed;
        }
        if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS) {
            translation += up * translation_speed;
        }
        if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) {
            translation -= up * translation_speed;
        }


        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            yaw += 1;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            yaw -= 1;
        }
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            pitch += 1;
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            pitch -= 1;
        }

        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction = glm::normalize(direction);

        const glm::vec3 curr_position = transform->get_position();
        transform->set_position(curr_position + translation);

        transform->look_towards(direction, up);
    }

    float FPSControl::get_translation_speed() const { return translation_speed; }

    float FPSControl::get_rotation_speed() const { return rotation_speed; }

    void FPSControl::set_translation_speed(const float translation_speed) {
        this->translation_speed = translation_speed;
    }

    void FPSControl::set_rotation_speed(const float rotation_speed) { this->rotation_speed = rotation_speed; }
} // namespace wrld::cpt
