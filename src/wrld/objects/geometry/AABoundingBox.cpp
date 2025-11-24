//
// Created by leo on 11/14/25.
//

#include <cfloat>
#include <wrld/objects/geometry/AABoundingBox.hpp>

namespace wrld::obj {
    AABoundingBox::AABoundingBox() :
        lower(glm::vec3{FLT_MAX}), upper(glm::vec3{-FLT_MAX}) {}

    AABoundingBox::AABoundingBox(const glm::vec3 &p1, const glm::vec3 &p2) {
        for (int i = 0; i < 3; i++) {
            this->lower[i] = std::min(p1[i], p2[i]);
            this->upper[i] = std::max(p1[i], p2[i]);
        }
    }

    glm::vec3 AABoundingBox::get_lower() const { return lower; }

    glm::vec3 AABoundingBox::get_upper() const { return upper; }

    glm::vec3 AABoundingBox::size() const { return upper - lower; }

    void AABoundingBox::add_point(const glm::vec3 &point) {
        for (int i = 0; i < 3; i++) {
            if (point[i] < lower[i])
                lower[i] = point[i];
            if (point[i] > upper[i])
                upper[i] = point[i];
        }
    }

    bool AABoundingBox::include(const glm::vec3 &point) const {
        for (int i = 0; i < 3; i++) {
            if (point[i] < lower[i] || point[i] > upper[i])
                return false;
        }
        return true;
    }

    glm::vec3 AABoundingBox::center() const { return (lower + upper) * 0.5f; }

    Box AABoundingBox::as_box() const {
        std::array<glm::vec3, 8> corners;
        corners[0] = lower;
        corners[1] = {upper.x, lower.y, lower.z};
        corners[2] = {lower.x, lower.y, upper.z};
        corners[3] = {upper.x, lower.y, upper.z};
        corners[4] = {lower.x, upper.y, lower.z};
        corners[5] = {upper.x, upper.y, lower.z};
        corners[6] = {lower.x, upper.y, upper.z};
        corners[7] = upper;

        return Box(corners);
    }

    AABoundingBox AABoundingBox::operator+(const AABoundingBox &other) const {
        glm::vec3 n_lower = lower;
        glm::vec3 n_upper = upper;

        for (int i = 0; i < 3; i++) {
            if (other.lower[i] < lower[i])
                n_lower[i] = other.lower[i];
            if (other.upper[i] > upper[i])
                n_upper[i] = other.upper[i];
        }

        return AABoundingBox{n_lower, n_upper};
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
                if (orig[axis] < lower[axis] || orig[axis] > upper[axis]) {
                    return std::nullopt;
                }

                continue;
            }

            // Not colinear to the axis
            double local_close = (lower[axis] - orig[axis]) / dir[axis];
            double local_far = (upper[axis] - orig[axis]) / dir[axis];

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

            if (close > far) // no intersection
                return std::nullopt;
            if (far < 0) // behind the origin
                return std::nullopt;
        }

        if (close > ray.get_length()) // too far
            return std::nullopt;

        return close;
    }
} // namespace wrld::obj
