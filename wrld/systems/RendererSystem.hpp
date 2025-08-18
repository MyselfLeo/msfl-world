//
// Created by leo on 8/16/25.
//

#ifndef RENDERERSYSTEM_HPP
#define RENDERERSYSTEM_HPP
#include "System.hpp"
#include "components/CameraComponent.hpp"
#include "resources/Model.hpp"
#include "resources/Program.hpp"

namespace wrld {

    class RendererSystem final : public System {
    public:
        RendererSystem(World &world, GLFWwindow *window);
        ~RendererSystem() override;
        void exec() override;

        GLFWwindow *get_window() const;

    private:
        GLFWwindow *window;
        Program model_program;

        glm::mat4x4 get_entity_transform(EntityID id) const;

        /// Return the active camera component.
        /// For now it's the first found.
        std::optional<std::shared_ptr<CameraComponent>> get_camera() const;
        Model get_entity_model(EntityID id) const;

        void draw_model(const Model &model, const glm::mat4x4 &model_matrix, const glm::mat4x4 &view_matrix,
                        const glm::mat4x4 &projection_matrix) const;
        void draw_mesh(const Mesh &mesh) const;
    };
} // namespace wrld

#endif // RENDERERSYSTEM_HPP
