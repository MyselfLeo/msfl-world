//
// Created by leo on 8/13/25.
//

#ifndef CAMERACOMPONENT_HPP
#define CAMERACOMPONENT_HPP
#include "Component.hpp"
#include "glm/mat4x4.hpp"

namespace wrld {

    /// Attach a camera to the Entity.
    class CameraComponent final : public Component {
    public:
        CameraComponent(EntityID entity_id, World &world, float fov);

        float get_fov() const;
        void set_fov(float fov);

        /// Either returns the Transform attached to the Entity,
        /// or the identity transform.
        glm::mat4x4 get_view_matrix() const;
        glm::mat4x4 get_projection_matrix(unsigned width, unsigned height) const;

    private:
        float fov;
    };

} // namespace wrld

#endif // CAMERACOMPONENT_HPP
