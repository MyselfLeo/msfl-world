//
// Created by leo on 11/20/25.
//

#include <glm/detail/func_geometric.hpp>
#include <wrld/objects/geometry/Ray.hpp>
#include <wrld/objects/geometry/Mesh.hpp>

namespace wrld::obj {
    Ray::Ray(const glm::vec3 &origin, const glm::vec3 &direction, const float length) :
        origin(origin), direction(glm::normalize(direction)), length(length) {}

    Ray::Ray(const glm::vec3 &origin, const glm::vec3 &other) :
        Ray(origin, other - origin, glm::length(other - origin)) {}

    glm::vec3 Ray::at(float t) const { return origin + direction * t; }

    glm::vec3 Ray::operator()(const float t) const { return at(t); }

    glm::vec3 Ray::get_origin() const { return origin; }

    glm::vec3 Ray::get_direction() const { return direction; }

    float Ray::get_length() const { return length; }

    Mesh Ray::get_mesh() const {
        std::vector<Vertex> vertices;
        vertices.emplace_back(origin, glm::vec3{0}, glm::vec2{0}, glm::vec3{0, 1, 0});
        vertices.emplace_back(origin + direction * length, glm::vec3{0}, glm::vec2{0},
                              glm::vec3{0, 1, 0});

        static const std::vector<VertexID> elements{0, 1};

        Mesh mesh{PrimitiveType::Lines};
        mesh.set_vertices(vertices);
        mesh.set_elements(elements);

        return mesh;
    }

    void Ray::set_length(const float length) { this->length = length; }

    void Ray::set_origin(const glm::vec3 &origin) { this->origin = origin; }

    void Ray::set_direction(const glm::vec3 &direction) { this->direction = direction; }
} // namespace wrld::obj
