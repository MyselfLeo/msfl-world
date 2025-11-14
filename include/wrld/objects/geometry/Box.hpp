//
// Created by leo on 10/24/25.
//

#pragma once
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <array>

namespace wrld::obj {
    class Mesh;

    /// Box defined by 8 vertices (see Box::vertices()).
    /// Transforms (mat4x4 multiplication) can be applied
    /// to a Box like any glm object.
    class Box {
    public:
        /// Empty box (all corners at {0, 0, 0}).
        Box();

        /// Box with set corners.
        explicit Box(const std::array<glm::vec3, 8> &&corners);

        /// Returns an axis-aligned box based on lower & upper corners.
        static Box bounding_box(const glm::vec3 &lower, const glm::vec3 &upper);

        /// Returns the center of this box (average of all corners).
        [[nodiscard]] glm::vec3 center() const;

        // /// Returns a bounding-box (see Box::bounding_box) with "impossible" values
        // /// (FLT_MAX on lower, FLT_MIN on upper). This box will always be updated when
        // /// calling add_point (making its state valid again).
        // static Box empty_bounding_box();
        //
        // /// If the box is a bounding box (created by Box::bounding_box or
        // /// Box::empty_bounding_box and not transformed by a 4x4 matrix), update
        // /// its corners to also include the given point.
        // void add_point(const glm::vec3 &point);

        /// Returns the "lower" corner.
        /// Only makes sense if the box was created as a bounding box.
        [[nodiscard]] glm::vec3 lower() const;
        [[nodiscard]] glm::vec3 upper() const;

        /// Return the size of the box.
        [[nodiscard]] glm::vec3 size() const;

        /// Return each vertices of the box.
        ///   6           upper
        ///      +---------+
        ///     /|        /|
        /// 4  / |     5 / |
        ///   +---------+  |              y
        ///   |  +----- | -+              |  z
        ///   | / 2     | /  3            | /
        ///   |/        |/                |/
        ///   +---------+                 +--- x
        /// lower        1
        [[nodiscard]] const std::array<glm::vec3, 8> &vertices() const;

        /// Returns true if the point is inside the box.
        /// Only makes sense if the box was created as a bounding box.
        bool inside(const glm::vec3 &point) const;

        /// Transform matrix application on a box.
        Box operator*(const glm::mat4x4 &trsfrm) const;

        /// Given 2 bounding boxes, return the bounding box of the set.
        /// Only makes sense if the box was created as a bounding box.
        Box operator+(const Box &other) const;

        /// Return a mesh representing the edges of this box.
        Mesh get_mesh() const;

    private:
        std::array<glm::vec3, 8> corners;
    };

} // namespace wrld::obj
