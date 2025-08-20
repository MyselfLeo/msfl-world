//
// Created by leo on 8/16/25.
//

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "RendererSystem.hpp"

#include "Logs.hpp"
#include "components/StaticModel.hpp"
#include "components/Transform.hpp"

#include <format>
#include <iostream>

namespace wrld {
    RendererSystem::RendererSystem(World &world, GLFWwindow *window) :
        System(world), window(window), model_program(Program{DEFAULT_VERTEX_SHADER, DEFAULT_FRAGMENT_SHADER}) {}

    RendererSystem::~RendererSystem() = default;

    void RendererSystem::exec() {
        glClearColor(0.06f, 0.06f, 0.06f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        // Required for the camera
        int width, height;
        glfwGetWindowSize(window, &width, &height);

        // Query the camera. No rendering if not found
        if (const auto camera_cmpnt = get_camera()) {
            const auto &camera = camera_cmpnt.value();

            const glm::mat4x4 view_matrix = camera->get_view_matrix();
            const glm::mat4x4 projection_matrix = camera->get_projection_matrix(width, height);

            model_program.use();
            model_program.set_uniform("view", view_matrix);
            model_program.set_uniform("projection", projection_matrix);

            model_program.set_uniform("ambiant_color", glm::vec4(ambiant_light.color, 1.0));
            model_program.set_uniform("ambiant_strength", ambiant_light.strength);

            if (const auto plight_cpmnt = get_point_light()) {
                const auto &plight = plight_cpmnt.value();
                model_program.set_uniform("light_position", plight.position);
                model_program.set_uniform("light_color", glm::vec4{plight.color, 1.0});
            }

            // Render each Entity with a Model attached
            for (const std::vector model_entities = world.get_entities_with_component<cpt::StaticModel>();
                 const auto entity: model_entities) {
                glm::mat4x4 model_matrix = get_entity_transform(entity);

                const auto model_cmpnt = world.get_component_opt<cpt::StaticModel>(entity).value();
                const Model &model = model_cmpnt->get_model();

                // Actual draw call
                draw_model(model, model_matrix);
            }
        }

        glfwSwapBuffers(window);
    }

    GLFWwindow *RendererSystem::get_window() const { return window; }

    void RendererSystem::set_ambiant_light_color(const glm::vec3 &color) { ambiant_light.color = color; }

    void RendererSystem::set_ambiant_light_strength(const float strength) { ambiant_light.strength = strength; }

    glm::vec3 RendererSystem::get_ambiant_light_color() const { return ambiant_light.color; }

    float RendererSystem::get_ambiant_light_strength() const { return ambiant_light.strength; }

    glm::mat4x4 RendererSystem::get_entity_transform(const EntityID id) const {
        if (const auto transform_cmpnt = world.get_component_opt<cpt::Transform>(id))
            return transform_cmpnt.value()->model_matrix();
        return glm::mat4x4(1.0);
    }

    std::optional<std::shared_ptr<cpt::Camera>> RendererSystem::get_camera() const {
        if (const std::vector camera_entities = world.get_entities_with_component<cpt::Camera>();
            !camera_entities.empty())
            return world.get_component_opt<cpt::Camera>(camera_entities[0]);
        return std::nullopt;
    }

    std::optional<PointLight> RendererSystem::get_point_light() const {
        PointLight res;

        const EntityID entity = world.get_entities_with_component<cpt::PointLight>()[0];

        res.color = world.get_component<cpt::PointLight>(entity)->get_color();
        res.intensity = world.get_component<cpt::PointLight>(entity)->get_intensity();
        res.position = world.get_component<cpt::Transform>(entity)->get_position();

        return res;
    }

    Model RendererSystem::get_entity_model(const EntityID id) const {
        return world.get_component_opt<cpt::StaticModel>(id).value()->get_model();
    }

    void RendererSystem::draw_model(const Model &model, const glm::mat4x4 &model_matrix) const {
        model_program.set_uniform("model", model_matrix);

        std::vector<std::shared_ptr<MeshGraphNode>> node_stack;
        node_stack.push_back(model.get_root_mesh());

        // Draw meshes by visiting the tree in depth
        while (!node_stack.empty()) {
            const auto &meshes = node_stack.back()->meshes;
            const auto &children = node_stack.back()->children;
            node_stack.pop_back();

            for (auto &mesh: meshes) {
                this->draw_mesh(mesh);
            }

            // Add children to be processed
            node_stack.insert(node_stack.end(), children.begin(), children.end());
        }
    }

    void RendererSystem::draw_mesh(const Mesh &mesh) const {
        // Load texture uniforms to the shader
        int i = 0;

        // Add textures as uniforms (sampler2D)
        model_program.set_uniform("use_texture", !mesh.textures.empty());
        for (const auto &[name, texture]: mesh.textures) {
            texture->use(i);
            model_program.set_uniform(name, i);
            i += 1;
        }
        glActiveTexture(GL_TEXTURE0);

        glBindVertexArray(mesh.vao);
        glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }
} // namespace wrld
