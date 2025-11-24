//
// Created by leo on 11/20/25.
//

#pragma once

#include <glm/vec3.hpp>

namespace wrld::obj {
    class Mesh;

    /// Half-line in space. Used for interesection tests.
    class Ray {
    public:
        /// Build a ray of a given origin & direction and a maximum length.
        /// The length is used to add a limit in intersection tests.
        /// The direction is always normalized.
        Ray(const glm::vec3 &origin, const glm::vec3 &direction, float length);

        /// Build a ray as a segment between 2 points. The first one will be the
        /// origin and the second one will be used to compute direction & length.
        /// The direction is always normalized.
        Ray(const glm::vec3 &origin, const glm::vec3 &other);

        /// Return P = origin + t * direction
        [[nodiscard]] glm::vec3 at(float t) const;

        /// See Ray::at().
        [[nodiscard]] glm::vec3 operator()(float t) const;

        [[nodiscard]] glm::vec3 get_origin() const;
        [[nodiscard]] glm::vec3 get_direction() const;
        [[nodiscard]] float get_length() const;

        /// Return a line mesh representing this ray.
        /// Its local coordinates will be the one of the mesh.
        [[nodiscard]] Mesh get_mesh() const;

        void set_origin(const glm::vec3 &origin);
        void set_direction(const glm::vec3 &direction);
        void set_length(float length);

    private:
        glm::vec3 origin;
        glm::vec3 direction;
        float length;
    };
} // namespace wrld::obj
