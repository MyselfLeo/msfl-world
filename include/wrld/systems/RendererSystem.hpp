//
// Created by leo on 8/16/25.
//

#pragma once

#include <wrld/System.hpp>
#include "GLFW/glfw3.h"
#include <wrld/components/Camera.hpp>
#include <wrld/components/Environment.hpp>
#include <wrld/resources/CubemapTexture.hpp>
#include <wrld/resources/Model.hpp>
#include <wrld/resources/Program.hpp>

namespace wrld {
    struct PointLightData {
        PointLightData(glm::vec3 position, glm::vec3 color, float intensity);

        glm::vec3 position;
        glm::vec3 color;
        float intensity;
    };

    struct DirectionalLightData {
        DirectionalLightData(glm::vec3 direction, glm::vec3 color, float intensity);

        glm::vec3 direction;
        glm::vec3 color;
        float intensity;
    };

    struct EnvironmentData {
        EnvironmentData(cpt::AmbiantLight ambiant_light, const std::optional<Rc<rsc::CubemapTexture>> &skybox,
                        GLuint vao);

        GLuint vao;
        cpt::AmbiantLight ambiant_light;
        std::optional<Rc<rsc::CubemapTexture>> skybox;
    };

    class RendererSystem : public System {
    public:
        static constexpr unsigned MAX_LIGHTS = 100;

        RendererSystem(World &world, GLFWwindow *window);
        ~RendererSystem() override;
        void exec() override;
        [[nodiscard]] GLFWwindow *get_window() const;

        /// Return the amount of models visible by the active camera.
        [[nodiscard]] unsigned get_visible_models() const;

    protected:
        GLFWwindow *window;

        Rc<rsc::Program> skybox_program;

        /// Amount of visible models on the active camera.
        unsigned visible_models = 0;

        /// Return the entity's transform or a default one if not provided.
        [[nodiscard]] glm::mat4x4 get_entity_transform(EntityID id) const;

        /// Return the active camera component (for now, the first CameraComponent found).
        /// Returns std::nullopt if there is none.
        [[nodiscard]] std::optional<std::shared_ptr<const cpt::Camera>> get_camera() const;

        /// Return the model of an entity. Fails if the entity has no StaticModel
        /// component.
        [[nodiscard]] Rc<rsc::Model> get_entity_model(EntityID id) const;

        virtual void render_camera(const cpt::Camera &camera);

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

        void draw_skybox(const rsc::CubemapTexture &cubemap, const cpt::Camera &camera, GLuint vao) const;

        static void draw_model(const rsc::Model &model, const glm::mat4x4 &model_matrix, const rsc::Program &program);
    };
} // namespace wrld
