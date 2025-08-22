//
// Created by leo on 8/20/25.
//

#ifndef DEBUGMOVE_HPP
#define DEBUGMOVE_HPP
#include "Component.hpp"
#include "GLFW/glfw3.h"

namespace wrld::cpt {

    class FPSControl final : public Component {
    public:
        FPSControl(EntityID entity_id, World &world);

        void update(GLFWwindow *window);

        float get_translation_speed() const;
        float get_rotation_speed() const;
        void set_translation_speed(float translation_speed);
        void set_rotation_speed(float rotation_speed);

    private:
        // We need to keep yaw and pitch
        float yaw = -90.0;
        float pitch = 0;

        float translation_speed = 0.1;
        float rotation_speed = 0.03;
    };

} // namespace wrld::cpt

#endif // DEBUGMOVE_HPP
