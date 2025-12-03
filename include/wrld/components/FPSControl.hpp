//
// Created by leo on 8/20/25.
//

#pragma once

#include <wrld/components/Component.hpp>
#include <wrld/components/Transform.hpp>
#include "GLFW/glfw3.h"

namespace wrld::cpt {

    class FPSControl final : public Component {
    public:
        using required_components = std::tuple<Transform>;

        FPSControl(EntityID entity_id, World &world);

        void update(GLFWwindow *window, float delta_time);

        [[nodiscard]] float get_translation_speed() const;
        [[nodiscard]] double get_camera_sensitivity() const;
        void set_translation_speed(float translation_speed);
        void set_camera_sensitivity(double camera_sensitivity);

        void enable_mouse_control(bool mouse_control);
        [[nodiscard]] bool is_mouse_control_enabled() const;

        void enable_keyboard_control(bool keyboard_control);
        [[nodiscard]] bool is_keyboard_control_enabled() const;

        static std::string get_type() { return "FPSControl"; }

    private:
        double last_mouse_x = 400;
        double last_mouse_y = 300;

        // We need to keep yaw and pitch
        double yaw = -90.0;
        double pitch = 0.0;

        float translation_speed = 10;
        double camera_sensitivity = 0.1;

        bool do_mouse_control = true;
        bool do_keyboard_control = true;
    };

} // namespace wrld::cpt
