//
// Created by leo on 8/13/25.
//

#ifndef CAMERACOMPONENT_HPP
#define CAMERACOMPONENT_HPP
#include <wrld/components/Component.hpp>
#include <wrld/resources/Program.hpp>
#include <wrld/resources/WindowFramebuffer.hpp>

#include <glm/mat4x4.hpp>

namespace wrld::cpt {

    /// Attach a camera to the Entity.
    class Camera final : public Component {
    public:
        explicit Camera(EntityID entity_id, World &world, float fov, std::shared_ptr<rsc::WindowFramebuffer> viewport,
                        Rc<rsc::Program> program);

        [[nodiscard]] float get_fov() const;
        void set_fov(float fov);

        [[nodiscard]] Rc<rsc::Program> get_program() const;
        void set_program(const Rc<rsc::Program> &program);

        /// Either returns the Transform attached to the Entity,
        /// or the identity transform.
        [[nodiscard]] glm::mat4x4 get_view_matrix() const;
        [[nodiscard]] glm::mat4x4 get_projection_matrix() const;
        [[nodiscard]] glm::mat4x4 get_viewport_matrix() const;

        /// Return the camera position in world space.
        /// This is directly related to the attached Transform component (if any).
        glm::vec3 get_position() const;

        // todo: add ortographic mode

        std::string get_type() override { return "Camera"; }

        // void load_default_resources() override;

    private:
        static const glm::vec3 UP_VECTOR;
        float fov;
        // std::shared_ptr<const rsc::Program> program;
        std::shared_ptr<const rsc::WindowFramebuffer> viewport;
    };

} // namespace wrld::cpt

#endif // CAMERACOMPONENT_HPP
