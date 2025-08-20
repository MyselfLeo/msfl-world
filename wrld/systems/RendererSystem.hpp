//
// Created by leo on 8/16/25.
//

#ifndef RENDERERSYSTEM_HPP
#define RENDERERSYSTEM_HPP
#include "System.hpp"
#include "components/Camera.hpp"
#include "components/PointLight.hpp"
#include "resources/Model.hpp"
#include "resources/Program.hpp"

namespace wrld {
    struct AmbiantLight {
        glm::vec3 color = {1.0, 1.0, 1.0};
        float strength = 0.4;
    };

    struct PointLight {
        glm::vec3 color;
        glm::vec3 position;
        float intensity;
    };

    class RendererSystem final : public System {
    public:
        RendererSystem(World &world, GLFWwindow *window);
        ~RendererSystem() override;
        void exec() override;
        GLFWwindow *get_window() const;

        void set_ambiant_light_color(const glm::vec3 &color);
        void set_ambiant_light_strength(float strength);
        [[nodiscard]] glm::vec3 get_ambiant_light_color() const;
        [[nodiscard]] float get_ambiant_light_strength() const;

    private:
        static constexpr auto DEFAULT_VERTEX_SHADER = "wrld/shaders/vertex/basic.glsl";
        static constexpr auto DEFAULT_FRAGMENT_SHADER = "wrld/shaders/fragment/basic.glsl";

        GLFWwindow *window;
        Program model_program;

        AmbiantLight ambiant_light;

        [[nodiscard]] glm::mat4x4 get_entity_transform(EntityID id) const;

        /// Return the active camera component (for now, the first CameraComponent found).
        /// Returns std::nullopt if there is none.
        [[nodiscard]] std::optional<std::shared_ptr<cpt::Camera>> get_camera() const;
        [[nodiscard]] std::optional<PointLight> get_point_light() const;
        [[nodiscard]] Model get_entity_model(EntityID id) const;

        void draw_model(const Model &model, const glm::mat4x4 &model_matrix) const;
        void draw_mesh(const Mesh &mesh) const;
    };
} // namespace wrld

#endif // RENDERERSYSTEM_HPP
