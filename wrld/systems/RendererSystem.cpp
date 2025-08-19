//
// Created by leo on 8/16/25.
//

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "RendererSystem.hpp"

#include "Logs.hpp"
#include "components/ModelComponent.hpp"
#include "components/TransformComponent.hpp"

#include <format>
#include <iostream>

namespace wrld {
    RendererSystem::RendererSystem(World &world, GLFWwindow *window) :
        System(world), window(window), model_program({"wrld/shaders/model.glsl"}) {}

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
            const auto &camera_ptr = camera_cmpnt.value();

            const glm::mat4x4 view_matrix = camera_ptr->get_view_matrix();
            const glm::mat4x4 projection_matrix = camera_ptr->get_projection_matrix(width, height);

            // Render each Entity with a Model attached
            for (const std::vector model_entities = world.get_entities_with_component<ModelComponent>();
                 const auto entity: model_entities) {
                glm::mat4x4 model_matrix = get_entity_transform(entity);

                const auto model_cmpnt = world.get_component_opt<ModelComponent>(entity).value();
                const Model &model = model_cmpnt->get_model();

                // Actual draw call
                draw_model(model, model_matrix, view_matrix, projection_matrix);
            }
        }

        glfwSwapBuffers(window);
    }

    GLFWwindow *RendererSystem::get_window() const { return window; }

    glm::mat4x4 RendererSystem::get_entity_transform(const EntityID id) const {
        if (const auto transform_cmpnt = world.get_component_opt<TransformComponent>(id))
            return transform_cmpnt.value()->model_matrix();
        return glm::mat4x4(1.0);
    }

    std::optional<std::shared_ptr<CameraComponent>> RendererSystem::get_camera() const {
        if (const std::vector camera_entities = world.get_entities_with_component<CameraComponent>();
            !camera_entities.empty())
            return world.get_component_opt<CameraComponent>(camera_entities[0]);
        return std::nullopt;
    }

    Model RendererSystem::get_entity_model(const EntityID id) const {
        return world.get_component_opt<ModelComponent>(id).value()->get_model();
    }

    void RendererSystem::draw_model(const Model &model, const glm::mat4x4 &model_matrix, const glm::mat4x4 &view_matrix,
                                    const glm::mat4x4 &projection_matrix) const {
        model_program.use();
        model_program.set_uniform("model", model_matrix);
        model_program.set_uniform("view", view_matrix);
        model_program.set_uniform("projection", projection_matrix);


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
        model_program.use();
        int i = 0;

        // Add textures as uniforms (sampler2D)
        model_program.set_uniform("use_texture", !mesh.textures.empty());
        for (const auto &[name, texture]: mesh.textures) {
            texture->use(i);
            try {
                model_program.set_uniform(name, i);
            } catch (const std::runtime_error &_) {
                // not doing anything. If the shader doesn't expect a particular
                // texture, it's its bad
            }
            i += 1;
        }
        glActiveTexture(GL_TEXTURE0);

        glBindVertexArray(mesh.vao);
        glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }
} // namespace wrld
