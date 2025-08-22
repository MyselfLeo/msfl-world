//
// Created by leo on 8/13/25.
//

#ifndef CAMERACOMPONENT_HPP
#define CAMERACOMPONENT_HPP
#include "Component.hpp"
#include "glm/mat4x4.hpp"

namespace wrld::cpt {

    /// Attach a camera to the Entity.
    class Camera final : public Component {
    public:
        Camera(EntityID entity_id, World &world, float fov);

        [[nodiscard]] float get_fov() const;
        void set_fov(float fov);

        /// Either returns the Transform attached to the Entity,
        /// or the identity transform.
        [[nodiscard]] glm::mat4x4 get_view_matrix() const;
        [[nodiscard]] glm::mat4x4 get_projection_matrix(unsigned width, unsigned height) const;

    private:
        static const glm::vec3 UP_VECTOR;
        float fov;

        /// Return the camera position in world space
        glm::vec3 get_position() const;
        /// Return the camera direction in world space
        glm::vec3 get_front_vec() const;
        /// Return the camera right vector in world space
        glm::vec3 get_right_vec() const;
        /// Return the vector pointing in the Y direction of the camera
        glm::vec3 get_up_vec() const;
    };

} // namespace wrld::cpt

#endif // CAMERACOMPONENT_HPP
