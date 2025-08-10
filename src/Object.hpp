//
// Created by leo on 8/10/25.
//

#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <glm/vec3.hpp>

#include "Mesh.hpp"
#include "glm/mat4x4.hpp"


namespace wrld {
    class Object {
    public:
        glm::vec3 position;
        // glm::vec3 rotation;
        glm::vec3 scale;
        Mesh mesh;

        glm::mat4x4 model_matrix() const;
    };
} // namespace wrld

#endif // OBJECT_HPP
