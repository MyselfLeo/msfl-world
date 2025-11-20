//
// Created by leo on 11/20/25.
//

#include <wrld/objects/geometry/Ray.hpp>

namespace wrld::obj {
    Ray::Ray(const glm::vec3 &origin, const glm::vec3 &direction) :
        origin(origin), direction(direction) {}

    glm::vec3 Ray::get_origin() const { return origin; }

    glm::vec3 Ray::get_direction() const { return direction; }

    void Ray::set_origin(const glm::vec3 &origin) { this->origin = origin; }

    void Ray::set_direction(const glm::vec3 &direction) { this->direction = direction; }
} // namespace wrld::obj
