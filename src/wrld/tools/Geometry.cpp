//
// Created by leo on 9/29/25.
//

#include <wrld/tools/Geometry.hpp>

namespace wrld::tools {
    bool Geometry::is_visible(World &world, const EntityID entity, const EntityID camera) {

        const auto &model_opt = world.get_component_opt<cpt::Transform>(entity);
        const auto model = model_opt.has_value() ? model_opt.value()->model_matrix() : glm::mat4x4(1.0);
        const auto &entity_model = world.get_component<cpt::StaticModel>(entity);
        const auto &camera_cpt = world.get_component<cpt::Camera>(camera);

        // Local-space axis-aligned bounding box of the model
        const auto &local_bb = entity_model->get_model()->get_local_bb();

        // Convert it to projection-space
        const auto &view = camera_cpt->get_view_matrix();
        const auto &proj = camera_cpt->get_projection_matrix();
        const auto trsfm = proj * view * model;

        // In projection space, the frustum is between -1 and 1 on each axis.
        // If any vertex of the bounding box is in the frustum, then the model is visible.
        for (const auto &v: local_bb.vertices()) {
            const glm::vec4 proj_vw = trsfm * glm::vec4(v, 1.0);
            const glm::vec3 proj_v = proj_vw / proj_vw.w;
            if (proj_v.x > -1 && proj_v.x < 1 && proj_v.y > -1 && proj_v.y < 1 && proj_v.z > -1 && proj_v.z < 1)
                return true;
        }

        return false;
    }
} // namespace wrld::tools
