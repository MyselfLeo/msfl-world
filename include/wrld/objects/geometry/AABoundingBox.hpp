//
// Created by leo on 11/14/25.
//

#pragma once
#include <wrld/objects/geometry/Box.hpp>
#include <wrld/objects/geometry/Ray.hpp>
#include <wrld/objects/geometry/GeoRequestable.hpp>

namespace wrld::obj {

    /// Box guarranted to be aligned to the axis of its local-space.
    ///    6           upper
    ///      +---------+
    ///     /|        /|
    /// 4  / |     5 / |
    ///   +---------+  |              y
    ///   |  +----- | -+              |  z
    ///   | / 2     | /  3            | /
    ///   |/        |/                |/
    ///   +---------+                 +--- x
    /// lower        1
    class AABoundingBox : public Box, public GeoRequestable {
    public:
        /// Empty bounding-box with its lower coordinates to FLT_MAX and
        /// its upper coordinates to -FLT_MAX.
        /// This ensure that operator+, add_point, etc. all works as predicted.
        AABoundingBox();

        /// Returns an axis-aligned bounding box based on lower & upper corners.
        AABoundingBox(const glm::vec3 &lower, const glm::vec3 &upper);

        /// Returns the "lower" corner of the bounding box (smallest in each axis).
        [[nodiscard]] glm::vec3 lower() const;

        /// Returns the "upper" corner of the bounding box (greatest in each axis).
        [[nodiscard]] glm::vec3 upper() const;

        /// Return the size of the box, in each axis.
        [[nodiscard]] glm::vec3 size() const;

        /// If the point is outside the bounding-box, the bounding box
        /// is expanded to fit the point.
        void add_point(const glm::vec3 &point);

        /// Given two bounding boxes, return the bounding box that encompasses both.
        AABoundingBox operator+(const AABoundingBox &other) const;

        [[nodiscard]] AABoundingBox get_bounding_box() const override;

        [[nodiscard]] std::optional<double> intersect(const Ray &ray) const override;

        [[nodiscard]] bool include(const glm::vec3 &point) const override;


    private:
        /// Recompute the corners to match lower & upper vertices.
        void recompute_corners(const glm::vec3 &lower, const glm::vec3 &upper);
    };

} // namespace wrld::obj
