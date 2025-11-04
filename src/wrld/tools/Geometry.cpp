//
// Created by leo on 9/29/25.
//

#include <wrld/tools/Geometry.hpp>

namespace wrld::tools {
    bool Geometry::is_visible(World &world, const EntityID entity,
                              const EntityID camera) {
        const auto &model_opt = world.get_component_opt<cpt::Transform>(entity);
        const auto model = model_opt.has_value() ? model_opt.value()->model_matrix()
                                                 : glm::mat4x4(1.0);
        const auto &entity_model = world.get_component<cpt::StaticModel>(entity);
        const auto &camera_cpt = world.get_component<cpt::Camera3D>(camera);

        // We must find a plane that totally separates the frustum and the bounding box.
        // For that, we test in 2 different spaces: the local-space (where the object's bb
        // is aligned to the axis) and the projective-space (where the frustum is aligned
        // to the axis). For each space, and each axis, we check if all the points of the
        // non-aligned box are on the same side of the plane. If so, then we can say early
        // that it is not visible. If no plane separates them, we can say that they are
        // indeed crossing. NB: It looks like the Separate-Axis theorem used in 2D

        // Local-space axis-aligned bounding box of the model
        const auto &local_bb = entity_model->get_model()->get_bounding_box();

        // Projective-space axis-aligned bounding box of the frustum
        static const obj::Box proj_frustum =
                obj::Box::bounding_box({-1, -1, 0}, {1, 1, 1});

        // Projective-space bounding box of the model
        const auto &view = camera_cpt->get_view_matrix();
        const auto &proj = camera_cpt->get_projection_matrix();
        const auto &transform = proj * view * model;
        const obj::Box proj_bb = local_bb * transform;

        // Local-space bounding box of the frustum
        const obj::Box local_frustum = proj_frustum * glm::inverse(transform);

        /// Check that all the points of box are greater/lower than the given point on the
        /// same axis.
        auto are_all_exterior = [](const obj::Box &box, const glm::vec3 &point,
                                   const bool negative = false) {
            for (int axis = 0; axis < 3; axis++) {
                bool all_exterior = true;
                for (const auto &p: box.vertices()) {
                    // They all need to be less than point
                    if (negative && p[axis] > point[axis]) {
                        all_exterior = false;
                        break;
                    }
                    // They all need to be greater than point
                    if (!negative && p[axis] < point[axis]) {
                        all_exterior = false;
                        break;
                    }
                }
                if (all_exterior)
                    return true;
            }
            return false;
        };

        /// Tries to find a plane (based on the axis-aligned bounding box) that
        /// separates the 2 bounding boxes.
        /// Only tests axis-aligned planes, so you need 2 tests to be sure (one
        /// in each space where the bounding boxes are axis-aligned).
        /// home-bb: the Bounding box that is aligned with the axis
        /// away-bb: the one that isnt.
        auto bb_collide = [&](const obj::Box &home_bb, const obj::Box &away_bb) {
            return !are_all_exterior(away_bb, home_bb.lower(), true) &&
                   !are_all_exterior(away_bb, home_bb.upper(), false);
        };

        // First test: in the projective-space
        if (!bb_collide(proj_frustum, proj_bb)) {
            // wrldInfo("No collide in projective space");
            return false;
        }

        // Second test: in the local-space
        if (!bb_collide(local_bb, local_frustum)) {
            // wrldInfo("No collide in local space");
            return false;
        }

        return true;
        // if (are_all_exterior(proj_bb, {-1, -1, -1}, true) ||
        //     are_all_exterior(proj_bb, {1, 1, 1}, false)) {
        //     return false;
        // }
        //
        // // Second test: in the local-space
        // if (are_all_exterior(local_frustum, ))


        // In projection space, the frustum is between -1 and 1 on each axis.
        // If any vertex of the bounding box is in the frustum, then the model is
        // visible. for (const auto &v: local_bb.vertices()) {
        //     const glm::vec4 proj_vw = trsfm * glm::vec4(v, 1.0);
        //     const glm::vec3 proj_v = proj_vw / proj_vw.w;
        //     if (proj_v.x >= -1 && proj_v.x <= 1 && proj_v.y >= -1 && proj_v.y <= 1
        //     && proj_v.z >= -1 && proj_v.z <= 1)
        //         return true;
        // }

        // We try to find a plane that separate the bounding box and the frustum.
        // The frustum is a box from -1 to 1 in each axis, so we can test if all the
        // points in the BB are "

        // return false;
    }
} // namespace wrld::tools
