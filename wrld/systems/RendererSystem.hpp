//
// Created by leo on 8/16/25.
//

#ifndef RENDERERSYSTEM_HPP
#define RENDERERSYSTEM_HPP
#include "System.hpp"
#include "GLFW/glfw3.h"
#include "components/Camera.hpp"
#include "components/Environment.hpp"
#include "components/PointLight.hpp"
#include "resources/CubemapTexture.hpp"
#include "resources/Model.hpp"
#include "resources/Program.hpp"

namespace wrld {
    struct PointLightData {
        PointLightData(glm::vec3 position, glm::vec4 color, float intensity);

        glm::vec3 position;
        glm::vec4 color;
        float intensity;
    };

    struct DirectionalLightData {
        DirectionalLightData(glm::vec3 direction, glm::vec4 color);

        glm::vec3 direction;
        glm::vec4 color;
    };

    struct EnvironmentData {
        EnvironmentData(cpt::AmbiantLight ambiant_light, const std::optional<std::shared_ptr<CubemapTexture>> &skybox,
                        GLuint vao);

        GLuint vao;
        cpt::AmbiantLight ambiant_light;
        std::optional<std::shared_ptr<CubemapTexture>> skybox;
    };

    class RendererSystem final : public System {
    public:
        static constexpr unsigned MAX_LIGHTS = 10;

        RendererSystem(World &world, GLFWwindow *window);
        ~RendererSystem() override;
        void exec() override;
        [[nodiscard]] GLFWwindow *get_window() const;

    private:
        static constexpr auto DEFAULT_VERTEX_SHADER = "wrld/shaders/vertex/basic.glsl";
        static constexpr auto DEFAULT_FRAGMENT_SHADER = "wrld/shaders/fragment/basic.glsl";

        static constexpr auto SKYBOX_VERTEX_SHADER = "wrld/shaders/vertex/skybox.glsl";
        static constexpr auto SKYBOX_FRAGMENT_SHADER = "wrld/shaders/fragment/skybox.glsl";

        GLFWwindow *window;
        Program model_program;
        Program skybox_program;

        /// Return the entity's transform or a default one if not provided.
        [[nodiscard]] glm::mat4x4 get_entity_transform(EntityID id) const;

        /// Return the active camera component (for now, the first CameraComponent found).
        /// Returns std::nullopt if there is none.
        [[nodiscard]] std::optional<std::shared_ptr<cpt::Camera>> get_camera() const;

        /// Return the model of an entity. Fails if the entity has no StaticModel
        /// component.
        [[nodiscard]] Model get_entity_model(EntityID id) const;

        void render_camera(const cpt::Camera &camera) const;

        /// Return the environment attached to the camera, or a default one if not provided.
        [[nodiscard]] EnvironmentData get_environment(const cpt::Camera &camera) const;

        /// Return data of all PointLights in the world.
        /// Won't return more than MAX_LIGHTS. The vector is resized before
        /// being returned.
        [[nodiscard]] std::vector<PointLightData> get_point_lights() const;

        /// Return data of all DirectionalLights in the world.
        /// Won't return more than MAX_LIGHTS. The vector is resized before
        /// being returned.
        [[nodiscard]] std::vector<DirectionalLightData> get_directional_lights() const;

        void draw_skybox(const CubemapTexture &cubemap, const cpt::Camera &camera, GLuint vao) const;
        void draw_model(const Model &model, const glm::mat4x4 &model_matrix) const;
        void draw_mesh(const Mesh &mesh) const;
    };
} // namespace wrld

#endif // RENDERERSYSTEM_HPP
