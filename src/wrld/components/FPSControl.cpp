//
// Created by leo on 8/20/25.
//

#include <wrld/components/FPSControl.hpp>
#include <wrld/components/Transform.hpp>
#include <wrld/World.hpp>

#include <iostream>

namespace wrld::cpt {
    FPSControl::FPSControl(const EntityID entity_id, World &world) :
        Component(entity_id, world) {}

    void FPSControl::update(GLFWwindow *window, float delta_time) {
        // Get the entity's transform first.
        // We don't have to do anything if it no longer has one
        const auto transform_opt = world.get_component_opt<Transform>(entity_id);
        if (!transform_opt.has_value())
            return;
        const auto transform = transform_opt.value();

        const auto up = glm::vec3{0, 1, 0};

        if (do_keyboard_control) {
            auto translation = glm::vec3{0.0};
            float speed = (translation_speed * delta_time);

            if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
                speed *= 6;
            }

            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
                translation += transform->forward() * speed;
            }
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
                translation -= transform->forward() * speed;
            }
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
                translation -=
                        glm::normalize(glm::cross(transform->forward(), up)) *
                        speed;
            }
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
                translation +=
                        glm::normalize(glm::cross(transform->forward(), up)) *
                        speed;
            }
            if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
                translation += up * speed;
            }
            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
                translation -= up * speed;
            }

            const glm::vec3 curr_position = transform->get_position();
            transform->set_position(curr_position + (translation));
        }

        if (do_mouse_control) {
            double mouse_x, mouse_y;
            glfwGetCursorPos(window, &mouse_x, &mouse_y);

            const double offset_x = mouse_x - last_mouse_x;
            const double offset_y = last_mouse_y - mouse_y;

            last_mouse_x = mouse_x;
            last_mouse_y = mouse_y;

            yaw += offset_x * camera_sensitivity;
            pitch += offset_y * camera_sensitivity;

            if (pitch > 89.0f)
                pitch = 89.0f;
            if (pitch < -89.0f)
                pitch = -89.0f;

            glm::vec3 direction;
            direction.x = std::cos(glm::radians(yaw)) * cos(glm::radians(pitch));
            direction.y = std::sin(glm::radians(pitch));
            direction.z = std::sin(glm::radians(yaw)) * cos(glm::radians(pitch));
            direction = glm::normalize(direction);

            transform->look_towards(direction, up);
        }
    }

    float FPSControl::get_translation_speed() const { return translation_speed; }

    double FPSControl::get_camera_sensitivity() const { return camera_sensitivity; }

    void FPSControl::set_translation_speed(const float translation_speed) {
        this->translation_speed = translation_speed;
    }

    void FPSControl::set_camera_sensitivity(const double camera_sensitivity) {
        this->camera_sensitivity = camera_sensitivity;
    }

    void FPSControl::enable_mouse_control(const bool mouse_control) {
        do_mouse_control = mouse_control;
    }

    bool FPSControl::is_mouse_control_enabled() const { return do_mouse_control; }

    void FPSControl::enable_keyboard_control(const bool keyboard_control) {
        do_keyboard_control = keyboard_control;
    }

    bool FPSControl::is_keyboard_control_enabled() const { return do_keyboard_control; }
} // namespace wrld::cpt
