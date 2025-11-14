//
// Created by leo on 10/24/25.
//

#include <wrld/objects/geometry/Box.hpp>
#include <wrld/objects/geometry/Mesh.hpp>

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

    Box::Box(const std::array<glm::vec3, 8> &&corners) : corners(corners) {}

    Box Box::bounding_box(const glm::vec3 &lower, const glm::vec3 &upper) {
        /*   6           upper
         *      +---------+
         *     /|        /|
         * 4  / |     5 / |
         *   +---------+  |              y
         *   |  +----- | -+              |  z
         *   | / 2     | /  3            | /
         *   |/        |/                |/
         *   +---------+                 +--- x
         * lower        1
         */
        return Box{{
                lower,
                {upper.x, lower.y, lower.z},
                {lower.x, lower.y, upper.z},
                {upper.x, lower.y, upper.z},
                {lower.x, upper.y, lower.z},
                {upper.x, upper.y, lower.z},
                {lower.x, upper.y, upper.z},
                upper,
        }};
    }

    glm::vec3 Box::center() const {
        glm::vec3 sum;
        for (const auto &c: corners) {
            sum += c;
        }
        return {sum.x / 8.0, sum.y / 8.0, sum.z / 8.0};
    }

    // Box Box::empty_bounding_box() {
    //     return bounding_box({FLT_MAX, FLT_MAX, FLT_MAX}, {FLT_MIN, FLT_MIN, FLT_MIN});
    // }
    //
    // void Box::add_point(const glm::vec3 &point) {
    //     glm::vec3 l = lower();
    //     glm::vec3 u = upper();
    //
    //
    // }

    glm::vec3 Box::lower() const { return corners[0]; }

    glm::vec3 Box::upper() const { return corners[7]; }

    glm::vec3 Box::size() const { return upper() - lower(); }

    const std::array<glm::vec3, 8> &Box::vertices() const { return corners; }

    bool Box::inside(const glm::vec3 &point) const {
        if (point.x < lower().x || point.x > upper().x)
            return false;
        if (point.y < lower().y || point.y > upper().y)
            return false;
        if (point.z < lower().z || point.z > upper().z)
            return false;
        return true;
    }

    Box Box::operator*(const glm::mat4x4 &trsfrm) const {
        Box res;
        for (int i = 0; i < 8; i++) {
            res.corners[i] = glm::vec4(corners[i], 1.0) * trsfrm;
        }
        return res;
    }

    Box Box::operator+(const Box &other) const {
        glm::vec3 lower = this->lower();
        glm::vec3 upper = this->upper();

        if (other.lower().x < lower.x)
            lower.x = other.lower().x;
        if (other.lower().y < lower.y)
            lower.y = other.lower().y;
        if (other.lower().z < lower.z)
            lower.z = other.lower().z;

        if (other.upper().x > upper.x)
            upper.x = other.upper().x;
        if (other.upper().y > upper.y)
            upper.y = other.upper().y;
        if (other.upper().z > upper.z)
            upper.z = other.upper().z;

        return bounding_box(lower, upper);
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
