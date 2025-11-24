//
// Created by leo on 10/24/25.
//

#pragma once
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <array>

namespace wrld::obj {
    class Mesh;
    class AABoundingBox;

    /// Box defined by 8 vertices (see Box::vertices()).
    class Box {
    public:
        virtual ~Box() = default;

        /// Empty box (all corners at {0, 0, 0}).
        Box();

        /// Box with set corners.
        explicit Box(const std::array<glm::vec3, 8> &corners);
        explicit Box(const std::array<glm::vec3, 8> &&corners);

        /// Returns the center of this box (average of all corners).
        [[nodiscard]] glm::vec3 center() const;

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

        /// Transform matrix application on a box.
        /// Instead of dividing the final result by the W component (operator*),
        /// we just discard it.
        [[nodiscard]] Box undivided_transform(const glm::mat4x4 &trsfrm) const;

        /// Transform matrix application on a box.
        /// Note that it automatically divides the components by w. If you don't
        /// want this division, Box::undivided_transform is what you need.
        Box operator*(const glm::mat4x4 &trsfrm) const;

        /// Return the coordinates of the corner i.
        glm::vec3 operator[](int idx) const;

        /// Return an axis-aligned bounding-box of this box.
        [[nodiscard]] virtual AABoundingBox get_bounding_box() const;

        /// Return a mesh representing the edges of this box.
        [[nodiscard]] Mesh get_mesh() const;

    protected:
        std::array<glm::vec3, 8> corners;
    };

} // namespace wrld::obj
