//
// Created by leo on 11/20/25.
//

#pragma once
#include <array>
#include <wrld/objects/geometry/AABoundingBox.hpp>
#include <wrld/objects/geometry/GeoRequestable.hpp>

namespace wrld::obj {

    /// A triangle made up of 3 points in counter-clockwise order.
    class Triangle : public GeoRequestable {
    public:
        Triangle(const glm::vec3 &v1, const glm::vec3 &v2, const glm::vec3 &v3);
        explicit Triangle(const std::array<glm::vec3, 3> &vertices);

        [[nodiscard]] AABoundingBox get_bounding_box() const override;

        [[nodiscard]] std::optional<double> intersect(const Ray &ray) const override;

        [[nodiscard]] bool include(const glm::vec3 &point) const override;

        [[nodiscard]] glm::vec3 center() const override;

        /// Return a line mesh representing this triangle.
        /// Its local coordinates will be the one of the mesh.
        [[nodiscard]] Mesh get_mesh() const;


        glm::vec3 &operator[](int idx);
        const glm::vec3 &operator[](int idx) const;

    private:
        std::array<glm::vec3, 3> vertices;
        AABoundingBox bb;
    };

} // namespace wrld::obj
