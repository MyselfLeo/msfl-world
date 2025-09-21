//
// Created by leo on 8/16/25.
//

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <wrld/systems/RendererSystem.hpp>
#include <wrld/components/Camera.hpp>
#include <wrld/components/DirectionalLight.hpp>
#include <wrld/components/StaticModel.hpp>
#include <wrld/components/Transform.hpp>
#include <wrld/components/Environment.hpp>
#include <wrld/components/PointLight.hpp>
#include <wrld/shaders/skybox_shader.hpp>

#include <format>
#include <iostream>
#include <wrld/logs.hpp>

namespace wrld {

    PointLightData::PointLightData(const glm::vec3 position, const glm::vec3 color, const float intensity) :
        position(position), color(color), intensity(intensity) {}

    DirectionalLightData::DirectionalLightData(const glm::vec3 direction, const glm::vec3 color,
                                               const float intensity) :
        direction(direction), color(color), intensity(intensity) {}

    EnvironmentData::EnvironmentData(const cpt::AmbiantLight ambiant_light,
                                     const std::optional<Rc<rsc::CubemapTexture>> &skybox, const GLuint vao) :
        vao(vao), ambiant_light(ambiant_light), skybox(skybox) {}

    RendererSystem::RendererSystem(World &world, GLFWwindow *window) : System(world), window(window) {
        const auto program = world.create_resource<rsc::Program>("skybox_program");
        program.get_mut()->from_source(shader::SKYBOX);
        skybox_program = program;
    }

    RendererSystem::~RendererSystem() = default;

    void RendererSystem::exec() {
        // Find the first camera in the world. It will be the render
        // one.
        // todo: in the future, each camera will be attached to a Viewport.
        // We'll have to render each camera to its attached viewport.

        if (const auto cameras = world.get_entities_with_component<cpt::Camera>(); !cameras.empty()) {
            const auto camera = world.get_component<cpt::Camera>(cameras[0]);
            render_camera(*camera);
        }
    }

    GLFWwindow *RendererSystem::get_window() const { return window; }

    glm::mat4x4 RendererSystem::get_entity_transform(const EntityID id) const {
        if (const auto transform_cmpnt = world.get_component_opt<cpt::Transform>(id))
            return transform_cmpnt.value()->model_matrix();
        return glm::mat4x4(1.0);
    }

    std::optional<std::shared_ptr<const cpt::Camera>> RendererSystem::get_camera() const {
        if (const std::vector camera_entities = world.get_entities_with_component<cpt::Camera>();
            !camera_entities.empty())
            return world.get_component_opt<cpt::Camera>(camera_entities[0]);
        return std::nullopt;
    }

    Rc<rsc::Model> RendererSystem::get_entity_model(const EntityID id) const {
        return world.get_component<cpt::StaticModel>(id)->get_model();
    }

    /*Program RendererSystem::get_entity_program(const EntityID id) const {
        const auto shdr = world.get_component_opt<cpt::Shader>(id);
        if (!shdr.has_value()) {
            return DEFAULT_PROGRAM;
        }
        return shdr.value()->get_program();
    }*/

    void RendererSystem::render_camera(const cpt::Camera &camera) const {
        const rsc::Program &program = camera.get_program().get_ref();

        // Todo: In the future, a camera should be attached to a viewport
        // of a given size. We should get this viewport size instead of the window size.
        // Of course, one of those viewports could be the window !
        int width, height;
        glfwGetWindowSize(window, &width, &height);

        const EnvironmentData environment_data = get_environment(camera);

        // Todo: When we will have multiple cameras, we should do that beforehand.
        // We don't need to call this each time we render a camera but once per exec().
        const std::vector<PointLightData> point_lights = get_point_lights();
        const std::vector<DirectionalLightData> directional_lights = get_directional_lights();

        if (environment_data.skybox.has_value()) {
            draw_skybox(environment_data.skybox.value().get_ref(), camera, environment_data.vao);
        }

        // Camera dependent uniforms
        const glm::mat4x4 view_matrix = camera.get_view_matrix();
        const glm::mat4x4 projection_matrix = camera.get_projection_matrix();
        program.use();
        program.set_uniform("view_pos", camera.get_position());
        program.set_uniform("view", view_matrix);
        program.set_uniform("projection", projection_matrix);

        // Ambiant light uniform
        program.set_uniform("ambiant_light.color", environment_data.ambiant_light.color);
        program.set_uniform("ambiant_light.intensity", environment_data.ambiant_light.intensity);

        // Point light dependent uniforms
        program.set_uniform("point_light_nb", static_cast<unsigned>(point_lights.size()));
        for (const auto &[i, pl]: std::views::enumerate(point_lights)) {
            program.set_uniform(std::format("point_lights[{}].position", i), pl.position);
            program.set_uniform(std::format("point_lights[{}].color", i), pl.color);
            program.set_uniform(std::format("point_lights[{}].intensity", i), pl.intensity);
        }

        // Directional light dependent uniforms
        program.set_uniform("directional_lights_nb", static_cast<unsigned>(directional_lights.size()));
        for (const auto &[i, dl]: std::views::enumerate(directional_lights)) {
            program.set_uniform(std::format("directional_lights[{}].direction", i), dl.direction);
            program.set_uniform(std::format("directional_lights[{}].color", i), dl.color);
            program.set_uniform(std::format("directional_lights[{}].intensity", i), dl.intensity);
        }

        // Find each entity with a model, get its transform, and render it.
        for (const std::vector model_entities = world.get_entities_with_component<cpt::StaticModel>();
             const auto entity: model_entities) {
            const auto model_cmpnt = world.get_component_opt<cpt::StaticModel>(entity).value();
            const auto &model = model_cmpnt->get_model();

            glm::mat4x4 model_matrix = get_entity_transform(entity);

            // Actual draw call
            draw_model(model.get_ref(), model_matrix, program);
        }
    }

    EnvironmentData RendererSystem::get_environment(const cpt::Camera &camera) const {
        const EntityID camera_entity = camera.get_entity();

        if (const auto env_cpnt_opt = world.get_component_opt<cpt::Environment>(camera_entity)) {
            const auto &env_cpnt = env_cpnt_opt.value();

            return EnvironmentData{env_cpnt->get_ambiant_light(), env_cpnt->get_cubemap(), env_cpnt->get_vao()};
        }

        return EnvironmentData{cpt::AmbiantLight{}, std::nullopt, 0};
    }

    std::vector<PointLightData> RendererSystem::get_point_lights() const {
        std::vector<PointLightData> res;

        // Query each PointLight components in world
        // Make sure we don't render more than 10 point lights (shader limitation)
        auto entities = world.get_entities_with_component<cpt::PointLight>();
        if (entities.size() > MAX_LIGHTS)
            entities.resize(MAX_LIGHTS);

        for (const auto &entity: entities) {
            const auto cpnt = world.get_component<cpt::PointLight>(entity);

            // We need the light's position. If not found, we use {0, 0, 0}.
            glm::vec3 position{0.0};
            if (const auto position_opt = world.get_component_opt<cpt::Transform>(entity); position_opt.has_value()) {
                position = position_opt.value()->get_position();
            }

            res.emplace_back(position, cpnt->get_color(), cpnt->get_intensity());
        }

        return res;
    }

    std::vector<DirectionalLightData> RendererSystem::get_directional_lights() const {
        std::vector<DirectionalLightData> res;

        // Query each DirectionalLight components in world
        // Make sure we don't render more than 10 directional lights (shader limitation)
        auto entities = world.get_entities_with_component<cpt::DirectionalLight>();
        if (entities.size() > MAX_LIGHTS)
            entities.resize(MAX_LIGHTS);

        for (const auto &entity: entities) {
            const auto cpnt = world.get_component<cpt::DirectionalLight>(entity);

            // We need the light's direction. If not found, we use {0, 0, -1}.
            glm::vec3 direction{0.0};
            if (const auto direction_opt = world.get_component_opt<cpt::Transform>(entity); direction_opt.has_value()) {
                direction = direction_opt.value()->get_direction();
            }

            res.emplace_back(direction, cpnt->get_color(), cpnt->get_intensity());
        }

        return res;
    }

    void RendererSystem::draw_skybox(const rsc::CubemapTexture &cubemap, const cpt::Camera &camera, GLuint vao) const {
        const auto &skybox_prgm = skybox_program.get_ref();
        skybox_prgm.use();

        const auto inv_matrix =
                glm::inverse(camera.get_viewport_matrix() * camera.get_projection_matrix() * camera.get_view_matrix());

        cubemap.use(0);

        skybox_prgm.set_uniform("inv_matrix", inv_matrix);
        skybox_prgm.set_uniform("camera_pos", camera.get_position());
        skybox_prgm.set_uniform("cubemap", 0);

        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);
        glDepthMask(GL_TRUE);
    }

    void RendererSystem::draw_model(const rsc::Model &model, const glm::mat4x4 &model_matrix,
                                    const rsc::Program &program) const {
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);

        program.set_uniform("model", model_matrix);

        // Compute the model matrix specific to normals
        const glm::mat4x4 normal_model_matrix = glm::transpose(glm::inverse(model_matrix));
        program.set_uniform("model_normal", normal_model_matrix);

        // todo: switch to multi draw call
        // Draw meshes material by material
        for (const auto &mat: model.get_materials()) {
            // meshes of this model with the given material
            const auto &meshes = mat.get_common_users(model.get_meshes());

            for (auto &mesh_name: meshes) {
                const auto &mesh = world.get_resource<rsc::Mesh>(mesh_name);
                draw_mesh(mesh.get_ref(), program);
            }
        }
    }

    void RendererSystem::draw_mesh(const rsc::Mesh &mesh, const rsc::Program &program) {
        program.set_uniform("material", mesh.get_material().get_ref());

        glActiveTexture(GL_TEXTURE0);

        glBindVertexArray(mesh.get_vao());

        glDrawElements(mesh.get_gl_primitive_type(), mesh.get_element_count(), GL_UNSIGNED_INT, nullptr);

        glBindVertexArray(0);
    }
} // namespace wrld
