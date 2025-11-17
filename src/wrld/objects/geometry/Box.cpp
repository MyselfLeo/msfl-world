//
// Created by leo on 10/24/25.
//

#include <wrld/objects/geometry/Box.hpp>
#include <wrld/objects/geometry/Mesh.hpp>
#include <wrld/objects/geometry/AABoundingBox.hpp>

namespace wrld::obj {
    Box::Box() :
        corners({{0, 0, 0},
                 {0, 0, 0},
                 {0, 0, 0},
                 {0, 0, 0},
                 {0, 0, 0},
                 {0, 0, 0},
                 {0, 0, 0},
                 {0, 0, 0}}) {}

    Box::Box(const std::array<glm::vec3, 8> &corners) : corners(corners) {}

    Box::Box(const std::array<glm::vec3, 8> &&corners) : corners(corners) {}

    glm::vec3 Box::center() const {
        glm::vec3 sum;
        for (const auto &c: corners) {
            sum += c;
        }
        return {sum.x / 8.0, sum.y / 8.0, sum.z / 8.0};
    }

    const std::array<glm::vec3, 8> &Box::vertices() const { return corners; }

    Box Box::undivided_transform(const glm::mat4x4 &trsfrm) const {
        Box res;
        for (int i = 0; i < 8; i++) {
            res.corners[i] = trsfrm * glm::vec4(corners[i], 1.0);
        }
        return res;
    }

    Box Box::operator*(const glm::mat4x4 &trsfrm) const {
        Box res;
        for (int i = 0; i < 8; i++) {
            glm::vec4 tmp = trsfrm * glm::vec4(corners[i], 1.0);
            tmp /= tmp.w;
            res.corners[i] = tmp;
        }
        return res;
    }

    glm::vec3 Box::operator[](const int idx) const { return corners[idx]; }

    AABoundingBox Box::get_bounding_box() const {
        AABoundingBox res{};
        for (const auto &v: vertices()) {
            res.add_point(v);
        }
        return res;
    }

    Mesh Box::get_mesh() const {
        std::vector<Vertex> vertices;
        for (const auto &c: corners) {
            vertices.emplace_back(c, glm::vec3{0}, glm::vec2{0}, glm::vec3{1, 0, 0});
        }

        // Order for the vertices
        //   6           upper
        //      +---------+
        //     /|        /|
        // 4  / |     5 / |
        //   +---------+  |              y
        //   |  +----- | -+              |  z
        //   | / 2     | /  3            | /
        //   |/        |/                |/
        //   +---------+                 +--- x
        // lower        1
        static const std::vector<VertexID> elements{
                0, 1, // 0
                0, 2, // 1
                0, 4, // 2
                1, 5, // 3
                1, 3, // 4,
                2, 3, // 5
                2, 6, // 6
                3, 7, // 7
                4, 5, // 8
                4, 6, // 9
                5, 7, // 10
                6, 7, // 11
        };

        Mesh mesh{PrimitiveType::Lines};
        mesh.set_vertices(vertices);
        mesh.set_elements(elements);

        return mesh;
    }

} // namespace wrld::obj
