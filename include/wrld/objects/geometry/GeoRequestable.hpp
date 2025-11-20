//
// Created by leo on 11/20/25.
//

#pragma once
#include <optional>

#include <glm/vec3.hpp>

namespace wrld::obj {

    class Ray;
    class AABoundingBox;

    class GeoRequestable {
    public:
        virtual ~GeoRequestable() = default;

        /// Returns the axis-aligned bounding box of the object.
        [[nodiscard]] virtual AABoundingBox get_bounding_box() const = 0;

        /// If the ray (half-line) interescts the object, returns k so that
        /// ray.origin + k * ray.direction is the interesection point.
        /// Returns nullopt otherwise.
        [[nodiscard]] virtual std::optional<double> intersect(const Ray &ray) const = 0;

        /// Returns true if the point is included in the object, false otherwise.
        /// The definition of "being inside" depends on the object.
        [[nodiscard]] virtual bool include(const glm::vec3 &point) const = 0;
    };

} // namespace wrld::obj
