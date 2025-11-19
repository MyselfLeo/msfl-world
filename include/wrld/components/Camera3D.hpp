//
// Created by leo on 8/13/25.
//

#pragma once

#include <wrld/components/Component.hpp>
#include <wrld/resources/Program.hpp>
#include <wrld/resources/WindowFramebuffer.hpp>

#include <glm/mat4x4.hpp>
#include <wrld/objects/geometry/Box.hpp>

namespace wrld::cpt {
    enum class ProjectionMode {
        Perspective,
        Orthographic,
    };

    /// Attach a camera to the Entity.
    class Camera3D final : public Component {
    public:
        /// Frustum in projective space.
        static const obj::AABoundingBox Frustum;

        explicit Camera3D(EntityID entity_id, World &world, float fov, bool do_culling,
                          std::shared_ptr<rsc::WindowFramebuffer> viewport,
                          const Rc<rsc::Program> &program);

        [[nodiscard]] float get_fov() const;

        void set_fov(float fov);

        [[nodiscard]] Rc<rsc::Program> get_program() const;

        void set_program(const Rc<rsc::Program> &program);

        /// Either returns the Transform attached to the Entity,
        /// or the identity transform.
        [[nodiscard]] glm::mat4x4 get_view_matrix() const;

        [[nodiscard]] glm::mat4x4 get_projection_matrix() const;

        [[nodiscard]] glm::mat4x4 get_viewport_matrix() const;

        [[nodiscard]] bool is_culling() const;

        void set_culling(bool do_culling);

        [[nodiscard]] float get_near_plane() const;
        [[nodiscard]] float get_far_plane() const;
        void set_near_plane(float z_near);
        void set_far_plane(float z_far);

        /// Return the camera position in world space.
        /// This is directly related to the attached Transform component (if any).
        glm::vec3 get_position() const;

        /// Return the frustum in camera local space.
        obj::Box get_frustum_box() const;

        /// Return the current projection mode.
        [[nodiscard]] ProjectionMode get_projection_mode() const;

        /// Defined the projection mode of this camera.
        void set_projection_mode(ProjectionMode projection_mode);

        // todo: add ortographic mode

        std::string get_type() override { return "Camera3D"; }

    private:
        static const glm::vec3 UP_VECTOR;
        float fov;
        bool do_culling;
        float near_plane = 0.1;
        float far_plane = 1000;
        std::shared_ptr<const rsc::WindowFramebuffer> viewport;
        ProjectionMode projection_mode;
    };
} // namespace wrld::cpt
