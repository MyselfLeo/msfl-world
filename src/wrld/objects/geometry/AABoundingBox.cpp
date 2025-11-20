//
// Created by leo on 11/14/25.
//

#include <cfloat>
#include <wrld/objects/geometry/AABoundingBox.hpp>

namespace wrld::obj {
    AABoundingBox::AABoundingBox() :
        AABoundingBox(glm::vec3{FLT_MAX}, glm::vec3{-FLT_MAX}) {}

    AABoundingBox::AABoundingBox(const glm::vec3 &lower, const glm::vec3 &upper) {
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

        recompute_corners(lower, upper);
    }

    glm::vec3 AABoundingBox::lower() const { return corners[0]; }

    glm::vec3 AABoundingBox::upper() const { return corners[7]; }

    glm::vec3 AABoundingBox::size() const { return upper() - lower(); }

    void AABoundingBox::add_point(const glm::vec3 &point) {
        glm::vec3 n_lower = lower();
        glm::vec3 n_upper = upper();

        if (point.x < n_lower.x)
            n_lower.x = point.x;
        if (point.y < n_lower.y)
            n_lower.y = point.y;
        if (point.z < n_lower.z)
            n_lower.z = point.z;

        if (point.x > n_upper.x)
            n_upper.x = point.x;
        if (point.y > n_upper.y)
            n_upper.y = point.y;
        if (point.z > n_upper.z)
            n_upper.z = point.z;

        recompute_corners(n_lower, n_upper);
    }

    bool AABoundingBox::include(const glm::vec3 &point) const {
        if (point.x < lower().x || point.x > upper().x)
            return false;
        if (point.y < lower().y || point.y > upper().y)
            return false;
        if (point.z < lower().z || point.z > upper().z)
            return false;
        return true;
    }

    AABoundingBox AABoundingBox::operator+(const AABoundingBox &other) const {
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

        return AABoundingBox{lower, upper};
    }

    AABoundingBox AABoundingBox::get_bounding_box() const { return *this; }

    std::optional<double> AABoundingBox::intersect(const Ray &ray) const {
        // Uses the slab method
        // See https://education.siggraph.org/static/HyperGraph/raytrace/rtinter3.htm

        const auto orig = ray.get_origin();
        const auto dir = ray.get_direction();

        double close = -DBL_MAX;
        double far = DBL_MAX;

        for (int axis = 0; axis < 3; axis++) {
            // if dir[axis] == 0, the ray is colinear to the axis.
            if (dir[axis] == 0) {
                // if it is parallel and the origin is not between the planes
                // then we are not intersecting for sure
                if (orig[axis] < lower()[axis] || orig[axis] > upper()[axis]) {
                    return false;
                }

                continue;
            }

            // Not colinear to the axis
            double local_close = (lower()[axis] - orig[axis]) / dir[axis];
            double local_far = (upper()[axis] - orig[axis]) / dir[axis];

            // We want local_close smaller than T2
            if (local_close > local_far) {
                std::swap(local_close, local_far);
            }

            if (local_close > close) {
                close = local_close;
            }
            if (local_far < far) {
                far = local_far;
            }

            if (close > far)
                return std::nullopt;
            if (far < 0)
                return std::nullopt;
        }

        return close;
    }

    void AABoundingBox::recompute_corners(const glm::vec3 &lower,
                                          const glm::vec3 &upper) {
        corners[0] = lower;
        corners[1] = {upper.x, lower.y, lower.z};
        corners[2] = {lower.x, lower.y, upper.z};
        corners[3] = {upper.x, lower.y, upper.z};
        corners[4] = {lower.x, upper.y, lower.z};
        corners[5] = {upper.x, upper.y, lower.z};
        corners[6] = {lower.x, upper.y, upper.z};
        corners[7] = upper;
    }
} // namespace wrld::obj
