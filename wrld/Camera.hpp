//
// Created by leo on 8/10/25.
//

#ifndef CAMERA_HPP
#define CAMERA_HPP
#include "GLFW/glfw3.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/quaternion.hpp"

namespace wrld {

    class Camera {
    public:
        explicit Camera(GLFWwindow *window);

        [[nodiscard]] glm::vec3 get_position() const;
        [[nodiscard]] glm::quat get_rotation() const;
        [[nodiscard]] float get_fov() const;

        [[nodiscard]] glm::mat4x4 view_matrix() const;
        [[nodiscard]] glm::mat4x4 projection_matrix() const;

        void set_position(const glm::vec3 &position);

        /// Set the camera rotation using a rotation quaternion.
        void set_rotation(const glm::quat &rotation);

        /// Set the camera rotation using Euler's angles (respectively pitch, yaw & roll).
        void set_rotation(const glm::vec3 &rotation);

        void set_fov(float fov);

    private:
        GLFWwindow *window;

        glm::vec3 position;
        glm::quat rotation;

        float fov;
    };

} // namespace wrld

#endif // CAMERA_HPP
