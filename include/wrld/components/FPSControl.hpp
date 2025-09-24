//
// Created by leo on 8/20/25.
//

#pragma once

#include <wrld/components/Component.hpp>
#include "GLFW/glfw3.h"

namespace wrld::cpt {

    class FPSControl final : public Component {
    public:
        FPSControl(EntityID entity_id, World &world);

        void update(GLFWwindow *window);

        [[nodiscard]] float get_translation_speed() const;
        [[nodiscard]] double get_camera_sensitivity() const;
        void set_translation_speed(float translation_speed);
        void set_camera_sensitivity(double camera_sensitivity);

        void set_lock(bool lock);
        [[nodiscard]] bool is_locked() const;

        std::string get_type() override { return "FPSControl"; }

    private:
        double last_mouse_x = 400;
        double last_mouse_y = 300;

        // We need to keep yaw and pitch
        double yaw = -90.0;
        double pitch = 0.0;

        float translation_speed = 0.2;
        double camera_sensitivity = 0.1;

        bool lock = false;
    };

} // namespace wrld::cpt
