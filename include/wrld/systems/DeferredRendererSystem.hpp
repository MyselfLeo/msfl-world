//
// Created by leo on 8/16/25.
//

#pragma once

#include <wrld/components/Camera.hpp>
#include <wrld/systems/RendererSystem.hpp>
#include <wrld/resources/DeferredFramebuffer.hpp>

namespace wrld {
    class DeferredRendererSystem : public RendererSystem {
    public:
        // static constexpr unsigned MAX_LIGHTS = 100;

        DeferredRendererSystem(World &world, GLFWwindow *window);


        ~DeferredRendererSystem() override;
        // void exec() override;
        // [[nodiscard]] GLFWwindow *get_window() const;

    private:
        GLuint vao;

        Rc<rsc::Program> pass1_program;
        Rc<rsc::Program> pass2_program;

        Rc<rsc::DeferredFramebuffer> framebuffer;

        // GLFWwindow *window;
        //
        // std::shared_ptr<const rsc::Program> skybox_program;
        //
        // /// Return the entity's transform or a default one if not provided.
        // [[nodiscard]] glm::mat4x4 get_entity_transform(EntityID id) const;
        //
        // /// Return the active camera component (for now, the first CameraComponent found).
        // /// Returns std::nullopt if there is none.
        // [[nodiscard]] std::optional<std::shared_ptr<const cpt::Camera>> get_camera() const;
        //
        // /// Return the model of an entity. Fails if the entity has no StaticModel
        // /// component.
        // [[nodiscard]] std::shared_ptr<const rsc::Model> get_entity_model(EntityID id) const;
        //
        // /*/// Return the program (shader) which shall be used to render an entity.
        // /// It's either the one defined using a cpt::Shader or the default one.
        // [[nodiscard]] Program get_entity_program(EntityID id) const;*/

        void render_camera(const cpt::Camera &camera) const override;

        // /// Return the environment attached to the camera, or a default one if not provided.
        // [[nodiscard]] EnvironmentData get_environment(const cpt::Camera &camera) const;
        //
        // /// Return data of all PointLights in the world.
        // /// Won't return more than MAX_LIGHTS. The vector is resized before
        // /// being returned.
        // [[nodiscard]] std::vector<PointLightData> get_point_lights() const;
        //
        // /// Return data of all DirectionalLights in the world.
        // /// Won't return more than MAX_LIGHTS. The vector is resized before
        // /// being returned.
        // [[nodiscard]] std::vector<DirectionalLightData> get_directional_lights() const;
        //
        // void draw_skybox(const rsc::CubemapTexture &cubemap, const cpt::Camera &camera, GLuint vao) const;
        //
        // void draw_model(const rsc::Model &model, const glm::mat4x4 &model_matrix, const rsc::Program &program) const;
        //
        // static void draw_mesh(const rsc::Mesh &mesh, const rsc::Program &program);
    };
} // namespace wrld
