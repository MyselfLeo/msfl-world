//
// Created by leo on 11/20/25.
//

#pragma once

#include <glm/vec3.hpp>

namespace wrld::obj {
    class Ray {
    public:
        Ray(const glm::vec3 &origin, const glm::vec3 &direction);

        [[nodiscard]] glm::vec3 get_origin() const;
        [[nodiscard]] glm::vec3 get_direction() const;

        void set_origin(const glm::vec3 &origin);
        void set_direction(const glm::vec3 &direction);

    private:
        glm::vec3 origin;
        glm::vec3 direction;
    };
} // namespace wrld::obj
