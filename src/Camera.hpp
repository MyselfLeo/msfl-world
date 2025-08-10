//
// Created by leo on 8/10/25.
//

#ifndef CAMERA_HPP
#define CAMERA_HPP
#include "glm/mat4x4.hpp"
#include "glm/vec3.hpp"

namespace wrld {

    class Camera {
    public:
        Camera(glm::vec3 position, glm::vec3 rotation);

        glm::vec3 position;
        glm::vec3 rotation;

        glm::mat4x4 view_matrix() const;
        glm::mat4x4 projection_matrix() const;
    };

} // namespace wrld

#endif // CAMERA_HPP
