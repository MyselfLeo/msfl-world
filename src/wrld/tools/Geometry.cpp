//
// Created by leo on 9/29/25.
//

#include <wrld/tools/Geometry.hpp>

namespace wrld::tools {
    bool Geometry::is_visible(World &world, const EntityID entity,
                              const EntityID camera) {
        /// Return true if there exists axis for which points of 'box' are either
        /// ALL greater or ALL lower than 'point'.
        auto all_outside = [](const obj::Box &box, const glm::vec3 &point,
                              const bool greater = true) {
            // Test all axis
            for (int axis = 0; axis < 3; axis++) {
                bool all = true;

                // Test all points
                for (const auto &p: box.vertices()) {
                    // They all need to be greater than point
                    if (greater && p[axis] < point[axis]) {
                        all = false;
                        break;
                    }
                    // They all need to be less than point
                    if (!greater && p[axis] > point[axis]) {
                        all = false;
                        break;
                    }
                }

                if (all)
                    return true;
            }

            return false;
        };

        /// Compares an AABB ("home_bb") and a regular bounding box ("away_bb").
        /// Returns false if it is certain that away_bb DOESN'T intersect home_bb.
        auto bb_collide = [&](const obj::AABoundingBox &home_bb,
                              const obj::Box &away_bb) {
            // lower and upper both represent 3 face planes of the home_bb
            // (6 in total as expected)
            if (all_outside(away_bb, home_bb.get_lower(), false)) {
                return false;
            }
            if (all_outside(away_bb, home_bb.get_upper(), true)) {
                return false;
            }
            return true;
        };

        const auto model_transform = world.get_component<cpt::Transform>(entity)->model_matrix();

        const auto &entity_model = world.get_component<cpt::StaticModel>(entity);
        const auto &camera_cpt = world.get_component<cpt::Camera3D>(camera);

        // Object's AABB in local space
        const auto &local_bb = entity_model->get_model()->get_bounding_box();

        // Frustum's corners in projective space
        const auto proj_frustum = cpt::Camera3D::Frustum;

        // Projective-space bounding box of the model
        const auto &view = camera_cpt->get_view_matrix();
        const auto &proj = camera_cpt->get_projection_matrix();
        const auto transform = proj * view * model_transform;

        const obj::Box proj_bb = local_bb.as_box() * transform;

        // First test : check in the projective-space
        if (!bb_collide(proj_frustum, proj_bb)) {
            return false;
        }

        // Local-space bounding box of the frustum
        const obj::Box local_frustum = proj_frustum.as_box() * glm::inverse(transform);

        // Second test : check in the local-space
        if (!bb_collide(local_bb, local_frustum)) {
            return false;
        }

        return true;
    }
} // namespace wrld::tools
