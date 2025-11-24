//
// Created by leo on 11/20/25.
//

#include <utility>
#include <wrld/objects/geometry/Triangle.hpp>
#include <wrld/objects/geometry/AABoundingBox.hpp>
#include <wrld/objects/geometry/Mesh.hpp>

namespace wrld::obj {
    Triangle::Triangle(const glm::vec3 &v1, const glm::vec3 &v2, const glm::vec3 &v3) :
        Triangle(std::array{v1, v2, v3}) {}

    Triangle::Triangle(const std::array<glm::vec3, 3> &vertices) : vertices(vertices) {
        bb = AABoundingBox{vertices[0], vertices[1]};
        bb.add_point(vertices[2]);
    }

    AABoundingBox Triangle::get_bounding_box() const { return bb; }

    std::optional<double> Triangle::intersect(const Ray &ray) const {
        const glm::vec3 e1 = vertices[1] - vertices[0];
        const glm::vec3 e2 = vertices[2] - vertices[0];

        const glm::vec3 pvec = glm::cross(ray.get_direction(), e2);
        const float inv_det = 1.0f / glm::dot(e1, pvec);
        const glm::vec3 tvec = ray.get_origin() - vertices[0];

        const float u = glm::dot(tvec, pvec) * inv_det;
        if (u < 0 || u > 1)
            return std::nullopt;

        const glm::vec3 qvec = glm::cross(tvec, e1);
        const float v = glm::dot(ray.get_direction(), qvec) * inv_det;
        if (v < 0 || u + v > 1)
            return std::nullopt;

        const float t = glm::dot(e2, qvec) * inv_det;
        if (t > ray.get_length() || t < 0)
            return std::nullopt;

        return t;
    }

    bool Triangle::include(const glm::vec3 &point) const {
        // fixme
        std::unreachable();
    }

    glm::vec3 Triangle::center() const {
        return (vertices[0] + vertices[1] + vertices[2]) / 3.0f;
    }

    Mesh Triangle::get_mesh() const {
        std::vector<Vertex> v;
        v.emplace_back(vertices[0], glm::vec3{0}, glm::vec2{0}, glm::vec3{0, 0, 1});
        v.emplace_back(vertices[1], glm::vec3{0}, glm::vec2{0}, glm::vec3{0, 0, 1});
        v.emplace_back(vertices[2], glm::vec3{0}, glm::vec2{0}, glm::vec3{0, 0, 1});

        static const std::vector<VertexID> elements{0, 1, 1, 2, 2, 0};

        Mesh mesh{PrimitiveType::Lines};
        mesh.set_vertices(v);
        mesh.set_elements(elements);

        return mesh;
    }

    glm::vec3 &Triangle::operator[](const int idx) { return vertices[idx]; }

    const glm::vec3 &Triangle::operator[](const int idx) const { return vertices[idx]; }
} // namespace wrld::obj
