//
// Created by leo on 8/13/25.
//

#include "glm/gtc/matrix_transform.hpp"

#include <wrld/components/Camera3D.hpp>
#include <wrld/components/Transform.hpp>
#include <wrld/World.hpp>
#include <wrld/objects/geometry/AABoundingBox.hpp>

namespace wrld::cpt {
    const obj::AABoundingBox Camera3D::Frustum =
            obj::AABoundingBox{{-1, -1, -1}, {1, 1, 1}};

    Camera3D::Camera3D(const EntityID entity_id, World &world, const float fov,
                       const bool do_culling,
                       std::shared_ptr<rsc::WindowFramebuffer> viewport,
                       const Rc<rsc::Program> &program) :
        Component(entity_id, world), fov(fov), do_culling(do_culling),
        viewport(std::move(viewport)) {
        attach_resource("program", program);
    }

    float Camera3D::get_fov() const { return fov; }

    void Camera3D::set_fov(const float fov) { this->fov = fov; }

    Rc<rsc::Program> Camera3D::get_program() const {
        return get_resource<rsc::Program>("program");
    }

    void Camera3D::set_program(const Rc<rsc::Program> &program) {
        attach_resource("program", program);
    }

    glm::mat4x4 Camera3D::get_view_matrix() const {
        return glm::inverse(world.get_component<Transform>(entity_id)->model_matrix());
    }

    glm::mat4x4 Camera3D::get_projection_matrix() const {
        if (projection_mode == ProjectionMode::Orthographic) {
            return glm::ortho(-fov, fov, -fov, fov, near_plane, far_plane);
        }

        const float ratio = static_cast<float>(viewport->get_width()) /
                            static_cast<float>(viewport->get_height());
        return glm::perspective(glm::radians(this->fov), ratio, near_plane, far_plane);
    }

    glm::mat4x4 Camera3D::get_viewport_matrix() const {
        const float w = viewport->get_width() / 2.f;
        const float h = viewport->get_height() / 2.f;

        return glm::mat4x4{w, 0, 0, 0, 0, h, 0, 0, 0, 0, 0.5, 0, w, h, 0.5, 1};
    }

    bool Camera3D::is_culling() const { return do_culling; }

    void Camera3D::set_culling(const bool do_culling) { this->do_culling = do_culling; }

    float Camera3D::get_near_plane() const { return near_plane; }

    float Camera3D::get_far_plane() const { return far_plane; }

    void Camera3D::set_near_plane(const float z_near) { near_plane = z_near; }

    void Camera3D::set_far_plane(const float z_far) { far_plane = z_far; }

    glm::vec3 Camera3D::get_position() const {
        return world.get_component<Transform>(entity_id)->get_position();
    }

    obj::Box Camera3D::get_frustum_box() const {
        std::array<glm::vec3, 8> corners;
        obj::Box frustum_box = Frustum.as_box();

        // Compute the reversed projected coordinates of each corner of the frustum
        for (int i = 0; i < 8; i++) {
            const auto point = glm::vec4{frustum_box[i], 1.0};

            const glm::vec4 view_space_corner =
                    glm::inverse(get_projection_matrix()) * point;

            corners[i] = glm::vec3{view_space_corner.x, view_space_corner.y,
                                   view_space_corner.z} /
                         view_space_corner.w;
        }

        return obj::Box(corners);
    }

    const glm::vec3 Camera3D::UP_VECTOR = glm::vec3(0, 1, 0);

    ProjectionMode Camera3D::get_projection_mode() const { return projection_mode; }

    void Camera3D::set_projection_mode(const ProjectionMode projection_mode) {
        this->projection_mode = projection_mode;
    }

    // void Camera::load_default_resources() {}
} // namespace wrld::cpt
