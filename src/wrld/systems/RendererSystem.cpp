//
// Created by leo on 8/16/25.
//

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <wrld/systems/RendererSystem.hpp>
#include <wrld/components/Camera3D.hpp>
#include <wrld/components/DirectionalLight.hpp>
#include <wrld/components/StaticModel.hpp>
#include <wrld/components/Transform.hpp>
#include <wrld/components/Environment3D.hpp>
#include <wrld/components/PointLight.hpp>
#include <wrld/shaders/skybox_shader.hpp>

#include <format>
#include <utility>
#include <wrld/Main.hpp>
#include <wrld/tools/Geometry.hpp>

#include "wrld/systems/NewRenderSystem.hpp"

namespace wrld {
    PointLightData::PointLightData(const glm::vec3 position, const glm::vec3 color,
                                   const float intensity) : position(position), color(color), intensity(intensity) {
    }

    DirectionalLightData::DirectionalLightData(const glm::vec3 direction,
                                               const glm::vec3 color,
                                               const float intensity) : direction(direction), color(color),
                                                                        intensity(intensity) {
    }

    EnvironmentData::EnvironmentData(const cpt::AmbiantLight ambiant_light,
                                     const std::optional<Rc<rsc::CubemapTexture> > &skybox,
                                     const GLuint vao) : vao(vao), ambiant_light(ambiant_light), skybox(skybox) {
    }

    RendererSystem::RendererSystem(World &world, GLFWwindow *window) : System(world), window(window) {
        skybox_program = world.create_resource<rsc::Program>("skybox_program");
        skybox_program->shader_source(rsc::ShaderType::Vertex, shader::SKYBOX);
        skybox_program->shader_source(rsc::ShaderType::Fragment, shader::SKYBOX);
        skybox_program->reload();

        frustum_culling_program = world.create_resource<rsc::Program>("frustum_culling_program");
        frustum_culling_program->shader_source(rsc::ShaderType::Compute, shader::FRUSTUM_CULLING);
        frustum_culling_program->reload();

        glGenBuffers(1, &model_matrices_buffer);
        glGenBuffers(1, &aabb_buffer);
        glGenBuffers(1, &result_buffer);
    }

    RendererSystem::~RendererSystem() = default;

    void RendererSystem::exec(const double delta_time) {
        // Find the first camera in the world. It will be the render
        // one.
        // todo: in the future, each camera will be attached to a Viewport.
        // We'll have to render each camera to its attached viewport.

        if (const auto cameras = world.get_entities_with_component<cpt::Camera3D>();
            !cameras.empty()) {
            const auto camera = world.get_component<cpt::Camera3D>(cameras[0]);
            render_camera(*camera);
        }
    }

    GLFWwindow *RendererSystem::get_window() const { return window; }

    unsigned RendererSystem::get_visible_models() const { return hidden_models; }

    glm::mat4x4 RendererSystem::get_entity_transform(const EntityID id) const {
        if (const auto transform_cmpnt = world.get_component_opt<cpt::Transform>(id))
            return transform_cmpnt.value()->model_matrix();
        return glm::mat4x4(1.0);
    }

    std::optional<std::shared_ptr<const cpt::Camera3D> >
    RendererSystem::get_camera() const {
        if (const std::vector camera_entities =
                    world.get_entities_with_component<cpt::Camera3D>();
            !camera_entities.empty())
            return world.get_component_opt<cpt::Camera3D>(camera_entities[0]);
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

    void RendererSystem::render_camera(const cpt::Camera3D &camera) {
        const auto program = camera.get_program();

        const EnvironmentData environment_data = get_environment(camera);

        // Todo: When we will have multiple cameras, we should do that beforehand.
        // We don't need to call this each time we render a camera but once per exec().
        const std::vector<PointLightData> point_lights = get_point_lights();
        const std::vector<DirectionalLightData> directional_lights =
                get_directional_lights();

        if (environment_data.skybox.has_value()) {
            draw_skybox(environment_data.skybox.value().get_ref(), camera,
                        environment_data.vao);
        }

        // Camera dependent uniforms
        const glm::mat4x4 view_matrix = camera.get_view_matrix();
        const glm::mat4x4 projection_matrix = camera.get_projection_matrix();
        program->use();
        program->set_uniform("view_pos", camera.get_position());
        program->set_uniform("view", view_matrix);
        program->set_uniform("projection", projection_matrix);

        // Ambiant light uniform
        program->set_uniform("ambiant_light.color", environment_data.ambiant_light.color);
        program->set_uniform("ambiant_light.intensity",
                             environment_data.ambiant_light.intensity);

        // Point light dependent uniforms
        program->set_uniform("point_light_nb",
                             static_cast<unsigned>(point_lights.size()));
        for (const auto &[i, pl]: std::views::enumerate(point_lights)) {
            program->set_uniform(std::format("point_lights[{}].position", i),
                                 pl.position);
            program->set_uniform(std::format("point_lights[{}].color", i), pl.color);
            program->set_uniform(std::format("point_lights[{}].intensity", i),
                                 pl.intensity);
        }

        // Directional light dependent uniforms
        program->set_uniform("directional_lights_nb",
                             static_cast<unsigned>(directional_lights.size()));
        for (const auto &[i, dl]: std::views::enumerate(directional_lights)) {
            program->set_uniform(std::format("directional_lights[{}].direction", i),
                                 dl.direction);
            program->set_uniform(std::format("directional_lights[{}].color", i),
                                 dl.color);
            program->set_uniform(std::format("directional_lights[{}].intensity", i),
                                 dl.intensity);
        }

        // Find each entity with a model, get its transform, and render it.
        unsigned visible_models = 0;
        const bool do_culling = camera.is_culling();
        const std::vector model_entities =
                world.get_entities_with_component<cpt::StaticModel>();
        for (const auto entity: model_entities) {
            // Skip unseen models if culling
            if (do_culling &&
                !tools::Geometry::is_visible(world, entity, camera.get_entity())) {
                continue;
            }

            visible_models += 1;

            const auto model_cmpnt = world.get_component<cpt::StaticModel>(entity);
            const auto &model = model_cmpnt->get_model();
            glm::mat4x4 model_matrix = get_entity_transform(entity);

            // Actual draw call
            draw_model(model.get_ref(), model_matrix, program.get_ref());
        }

        unsigned total_models = model_entities.size();
        Main::set_statistic("Visible model count (forward)",
                            std::format("{}/{}", visible_models, total_models));
    }

    EnvironmentData RendererSystem::get_environment(const cpt::Camera3D &camera) const {
        const EntityID camera_entity = camera.get_entity();

        if (const auto env_cpnt_opt =
                world.get_component_opt<cpt::Environment3D>(camera_entity)) {
            const auto &env_cpnt = env_cpnt_opt.value();

            return EnvironmentData{
                env_cpnt->get_ambiant_light(), env_cpnt->get_cubemap(),
                env_cpnt->get_vao()
            };
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
            if (const auto position_opt = world.get_component_opt<cpt::Transform>(entity);
                position_opt.has_value()) {
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
            if (const auto direction_opt =
                        world.get_component_opt<cpt::Transform>(entity);
                direction_opt.has_value()) {
                direction = direction_opt.value()->forward();
            }

            res.emplace_back(direction, cpnt->get_color(), cpnt->get_intensity());
        }

        return res;
    }

    void RendererSystem::draw_skybox(const rsc::CubemapTexture &cubemap,
                                     const cpt::Camera3D &camera, const GLuint vao) const {
        skybox_program->use();

        const auto inv_matrix =
                glm::inverse(camera.get_viewport_matrix() *
                             camera.get_projection_matrix() * camera.get_view_matrix());

        cubemap.use(0);

        skybox_program->set_uniform("inv_matrix", inv_matrix);
        skybox_program->set_uniform("camera_pos", camera.get_position());
        skybox_program->set_uniform("cubemap", 0);

        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);
        glDepthMask(GL_TRUE);
    }

    void RendererSystem::draw_model(const rsc::Model &model,
                                    const glm::mat4x4 &model_matrix,
                                    const rsc::Program &program) {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        program.set_uniform("model", model_matrix);

        // Compute the model matrix specific to normals
        const glm::mat4x4 normal_model_matrix =
                glm::transpose(glm::inverse(model_matrix));
        program.set_uniform("model_normal", normal_model_matrix);


        // Draw all meshes sharing the same primitive type & material
        // at the same time using glMultiDrawElements

        for (const auto &[pt, r]: model.get_mesh_ebo_data()) {
            for (const auto &[mat_idx, meshes]: r) {
                const auto &material = model.get_materials()[mat_idx];
                program.set_uniform("material", material);

                glDepthMask(material->is_doing_depth_mask());

                std::vector<int64_t> mat_starts{};
                mat_starts.reserve(meshes.size());
                std::vector<GLsizei> mat_sizes{};
                mat_sizes.reserve(meshes.size());

                GLenum primitive_type;
                switch (pt) {
                    case obj::PrimitiveType::Points: {
                        primitive_type = GL_POINTS;
                    }
                    break;
                    case obj::PrimitiveType::Lines: {
                        primitive_type = GL_LINES;
                    }
                    break;
                    case obj::PrimitiveType::Triangles: {
                        primitive_type = GL_TRIANGLES;
                    }
                    break;
                    default:
                        std::unreachable();
                }

                for (const auto [count, start]: meshes) {
                    mat_starts.push_back(start * sizeof(GLuint));
                    mat_sizes.push_back(count);
                } {
                    glBindVertexArray(model.get_vao());
                    if (material->get_polygon_mode() & rsc::WrldPolyFill) {
                        program.set_uniform("polygon_mode", 0);

                        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                        // glActiveTexture(GL_TEXTURE0);
                        glBindVertexArray(model.get_vao());

                        glMultiDrawElements(
                            primitive_type, mat_sizes.data(), GL_UNSIGNED_INT,
                            reinterpret_cast<const void **>(mat_starts.data()),
                            meshes.size());
                        glBindVertexArray(0);

                        glDepthMask(GL_FALSE);
                        glDepthFunc(GL_LEQUAL);
                    }

                    if (material->get_polygon_mode() & rsc::WrldPolyLine) {
                        program.set_uniform("polygon_mode", 1);

                        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                        glLineWidth(material->get_line_width());
                        // glActiveTexture(GL_TEXTURE0);
                        glBindVertexArray(model.get_vao());
                        glMultiDrawElements(
                            primitive_type, mat_sizes.data(), GL_UNSIGNED_INT,
                            reinterpret_cast<const void **>(mat_starts.data()),
                            meshes.size());
                        glBindVertexArray(0);

                        glDepthMask(GL_FALSE);
                        glDepthFunc(GL_LEQUAL);
                    }

                    if (material->get_polygon_mode() & rsc::WrldPolyPoint) {
                        program.set_uniform("polygon_mode", 2);

                        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
                        glPointSize(material->get_point_size());
                        // glActiveTexture(GL_TEXTURE0);
                        glBindVertexArray(model.get_vao());
                        glMultiDrawElements(
                            primitive_type, mat_sizes.data(), GL_UNSIGNED_INT,
                            reinterpret_cast<const void **>(mat_starts.data()),
                            meshes.size());
                        glBindVertexArray(0);
                    }

                    glDepthMask(GL_TRUE);
                    glDepthFunc(GL_LESS);
                    glBindVertexArray(0);
                }
            }
        }
    }
} // namespace wrld
