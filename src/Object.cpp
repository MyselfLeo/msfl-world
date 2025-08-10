//
// Created by leo on 8/10/25.
//

#include "Object.hpp"

#include "glm/gtx/transform.hpp"

namespace wrld {
    glm::mat4x4 Object::model_matrix() const { return glm::translate(this->position) * glm::scale(this->scale); }
} // namespace wrld
