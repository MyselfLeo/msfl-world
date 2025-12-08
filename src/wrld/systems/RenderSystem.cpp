//
// Created by leo on 12/8/25.
//

#include <wrld/systems/RenderSystem.hpp>

#include "wrld/Main.hpp"
#include "wrld/components/Camera3D.hpp"
#include "wrld/components/DirectionalLight.hpp"
#include "wrld/components/PointLight.hpp"
#include "wrld/components/StaticModel.hpp"

namespace wrld::sys {
    void RenderSystem::reload_resources(const World &world) {
        model_indices.clear();
        materials.clear();

        std::vector<StaticModelData> model_data;
        std::vector<StaticMeshData> mesh_data;

        const auto static_models = world.get_resources().at(std::type_index(typeid(rsc::Model))) | std::views::values;

        // 2 steps : first we collect the size of everything to allocate the
        // buffers then we fill them.
        // It prevents too much reallocation

        // Count number of vertices & elements
        // NB: elements = 1 value of the EBO (so 1 triangle = 3 elements)
        size_t vertex_count = 0;
        size_t element_count = 0;
        size_t total_mesh_count = 0;
        for (const auto &raw_model: static_models) {
            const auto &model = raw_model.as<rsc::Model>();
            for (const auto &[mg, _]: model->get_mesh_groups()) {
                for (const auto &mesh: mg.get_meshes()) {
                    total_mesh_count += 1;
                    vertex_count += mesh.get_vertex_count();
                    element_count += mesh.get_element_count();
                }
            }
        }

        std::vector<obj::Vertex> vertices;
        std::vector<obj::VertexID> elements;
        vertices.reserve(vertex_count);
        vertices.reserve(element_count);

        mesh_data.reserve(total_mesh_count);

        // Collect mesh data + materials
        for (const auto &raw_model: static_models) {
            const auto &model = raw_model.as<rsc::Model>();

            const GLuint mesh_start = mesh_data.size();
            GLuint mesh_count = 0;

            for (const auto &[mg, mat_id]: model->get_mesh_groups()) {
                // Material & textures
                const auto &mat = model->get_materials()[mat_id];
                unsigned global_material_index;
                if (auto i = std::ranges::find(materials, mat); i != materials.end()) {
                    global_material_index = i - materials.begin();
                } else {
                    global_material_index = materials.size();
                    materials.push_back(mat);
                }

                // Mesh data
                for (const auto &mesh: mg.get_meshes()) {
                    mesh_count += 1;

                    const GLuint vao_start = elements.size();
                    const GLuint vao_count = mesh.get_element_count();

                    const obj::VertexID offset = vertices.size();

                    vertices.insert(vertices.end(), mesh.get_vertices().begin(), mesh.get_vertices().end());

                    for (const auto &new_el: mesh.get_elements()) {
                        elements.push_back(new_el + offset);
                    }

                    mesh_data.emplace_back(
                        obj::get_primitive_gl_enum(mesh.get_primitive_type()),
                        global_material_index,
                        vao_start,
                        vao_count);
                }
            }

            model_indices.insert_or_assign(model, model_data.size());

            AABB bounding_box = {
                glm::vec4{model->get_bounding_box().get_lower(), 1.0},
                glm::vec4{model->get_bounding_box().get_upper(), 1.0}
            };

            model_data.emplace_back(
                bounding_box,
                mesh_start,
                mesh_count
            );
        }

        // Send data to the GPU.
        // First we send the geometry data (for vertex & fragment shaders),
        // then the information required by the compute shader,
        // then the materials.

        ///////////////// Geometry data

        glBindVertexArray(static_models_vao);
        glBindBuffer(GL_ARRAY_BUFFER, static_models_vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, static_models_ebo);

        // Usage is STATIC_DRAW as expected
        glBufferStorage(GL_ARRAY_BUFFER, vertices.size() * sizeof(obj::Vertex), vertices.data(), 0);
        glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, elements.size() * sizeof(unsigned), elements.data(), 0);

        // Vertex positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(obj::Vertex),
                              static_cast<void *>(nullptr));
        // Vertex normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(obj::Vertex),
                              reinterpret_cast<void *>(offsetof(obj::Vertex, normal)));
        // Vertex colors
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(obj::Vertex),
                              reinterpret_cast<void *>(offsetof(obj::Vertex, color)));
        // Vertex texture coordinates
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(obj::Vertex),
                              reinterpret_cast<void *>(offsetof(obj::Vertex, texcoords)));

        glBindVertexArray(0);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, model_data_buffer);
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(StaticModelData) * model_data.size(), model_data.data(), 0);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, mesh_data_buffer);
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(StaticMeshData) * mesh_data.size(), mesh_data.data(), 0);
    }

    void RenderSystem::set_scene_uniforms(const Rc<rsc::Program> &program,
                                          const cpt::AmbiantLight &ambiant_light,
                                          const LightCollection &lights) {
        program->use();

        // Ambiant light uniform
        program->set_uniform("ambiant_light.color", ambiant_light.color);
        program->set_uniform("ambiant_light.intensity", ambiant_light.intensity);

        // Point light dependent uniforms
        program->set_uniform("point_light_nb",
                             static_cast<unsigned>(lights.point_lights.size()));
        for (const auto &[i, pl]: std::views::enumerate(lights.point_lights)) {
            program->set_uniform(std::format("point_lights[{}].position", i),
                                 pl.position);
            program->set_uniform(std::format("point_lights[{}].color", i), pl.color);
            program->set_uniform(std::format("point_lights[{}].intensity", i),
                                 pl.intensity);
        }

        // Directional light dependent uniforms
        program->set_uniform("directional_lights_nb",
                             static_cast<unsigned>(lights.directional_lights.size()));
        for (const auto &[i, dl]: std::views::enumerate(lights.directional_lights)) {
            program->set_uniform(std::format("directional_lights[{}].direction", i),
                                 dl.direction);
            program->set_uniform(std::format("directional_lights[{}].color", i),
                                 dl.color);
            program->set_uniform(std::format("directional_lights[{}].intensity", i),
                                 dl.intensity);
        }
    }

    void RenderSystem::set_camera_uniforms(const Rc<rsc::Program> &program, const cpt::Camera3D &camera) {
        const glm::mat4x4 view_matrix = camera.get_view_matrix();
        const glm::mat4x4 projection_matrix = camera.get_projection_matrix();

        program->use();
        program->set_uniform("view_pos", camera.get_position());
        program->set_uniform("view", view_matrix);
        program->set_uniform("projection", projection_matrix);
    }

    void RenderSystem::bind_trsfm_buffer(const obj::PrimitiveType primitive_type) const {
        switch (primitive_type) {
            case obj::PrimitiveType::Points:
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, point_mesh_trsfm_buffer);
                break;
            case obj::PrimitiveType::Lines:
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, line_mesh_trsfm_buffer);
                break;
            case obj::PrimitiveType::Triangles:
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, triangle_mesh_trsfm_buffer);
                break;
        }
    }

    std::vector<RenderSystem::PointLightData> RenderSystem::get_point_lights(World &world) {
        std::vector<PointLightData> res;

        // Query each PointLight components in world
        // Make sure we don't render more than 10 point lights (shader limitation)
        auto entities = world.get_entities_with_component<cpt::PointLight>();
        if (entities.size() > MAX_LIGHTS)
            entities.resize(MAX_LIGHTS);

        for (const auto &entity: entities) {
            const auto cpnt = world.get_component<cpt::PointLight>(entity);
            glm::vec3 position = world.get_component<cpt::Transform>(entity)->get_position();
            res.emplace_back(position, cpnt->get_color(), cpnt->get_intensity());
        }

        return res;
    }

    std::vector<RenderSystem::DirectionalLightData> RenderSystem::get_directional_lights(World &world) {
        std::vector<DirectionalLightData> res;

        // Query each DirectionalLight components in world
        // Make sure we don't render more than 10 directional lights (shader limitation)
        auto entities = world.get_entities_with_component<cpt::DirectionalLight>();
        if (entities.size() > MAX_LIGHTS)
            entities.resize(MAX_LIGHTS);

        for (const auto &entity: entities) {
            const auto cpnt = world.get_component<cpt::DirectionalLight>(entity);
            glm::vec3 direction = world.get_component<cpt::Transform>(entity)->forward();
            res.emplace_back(direction, cpnt->get_color(), cpnt->get_intensity());
        }

        return res;
    }

    void RenderSystem::compute_renderables_visiblity(World &world, const cpt::Camera3D &camera) {
        // Find all entities that should be rendered
        const std::vector<EntityID> visible_objects = world.get_entities_with_component<cpt::StaticModel>();
        std::vector<Renderable> renderables;
        renderables.reserve(visible_objects.size());
        renderable_count = visible_objects.size();

        max_mesh_count = {0, 0, 0};

        for (const auto entity: visible_objects) {
            const auto transform_cpt = world.get_component<cpt::Transform>(entity);
            const auto model_cpt = world.get_component<cpt::StaticModel>(entity);
            const auto model = model_cpt->get_model();

            renderables.emplace_back(transform_cpt->model_matrix(), model_indices[model]);

            for (const auto &mg: model->get_mesh_groups() | std::views::keys) {
                switch (mg.get_primitive_type()) {
                    case obj::PrimitiveType::Points:
                        std::get<0>(max_mesh_count) += mg.get_mesh_count();
                        break;
                    case obj::PrimitiveType::Lines:
                        std::get<1>(max_mesh_count) += mg.get_mesh_count();
                        break;
                    case obj::PrimitiveType::Triangles:
                        std::get<2>(max_mesh_count) += mg.get_mesh_count();
                        break;
                }
            }
        }

        // Send data to GPU
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, renderable_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Renderable) * renderable_count, renderables.data(),
                     GL_DYNAMIC_DRAW);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, visibility_buffer);
        // Can't use GLboolean as it's 1 byte, not 4 like on GPU
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * renderable_count, nullptr,
                     GL_DYNAMIC_DRAW);

        // Prepare the dispatch
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, model_data_buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, renderable_buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, visibility_buffer);

        visibility_program->use();
        visibility_program->set_uniform("view_matrix", camera.get_view_matrix());
        visibility_program->set_uniform("proj_matrix", camera.get_projection_matrix());

        // Execute the compute shader
        static constexpr unsigned GROUP_SIZE_X = 256; // Should be the same than GPU side
        glDispatchCompute((renderable_count + GROUP_SIZE_X - 1) / GROUP_SIZE_X, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_COMMAND_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);

        // Debug
        std::vector<unsigned> visibility(renderable_count);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, visibility_buffer);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned) * renderable_count, visibility.data());
        unsigned count = 0;
        for (const auto b: visibility)
            if (b) count += 1;

        Main::set_statistic("Visible renderable", std::to_string(count));
    }

    void RenderSystem::compute_draw_calls_for_material(const unsigned material_idx) const {
        // Input
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, model_data_buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mesh_data_buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, renderable_buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, visibility_buffer);

        // Output of the shader, this is what we have to fill beforehand
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, arb_counter_buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, points_indirect_draw_buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, lines_indirect_draw_buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, triangles_indirect_draw_buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, point_mesh_trsfm_buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, line_mesh_trsfm_buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, triangle_mesh_trsfm_buffer);

        // Send data to the GPU
        std::array<GLuint, 3> counter{0, 0, 0};
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, arb_counter_buffer);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint) * counter.size(), counter.data());

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, points_indirect_draw_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(DrawElementsIndirectCommand) * std::get<0>(max_mesh_count),
                     nullptr,
                     GL_DYNAMIC_DRAW);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, lines_indirect_draw_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(DrawElementsIndirectCommand) * std::get<1>(max_mesh_count),
                     nullptr,
                     GL_DYNAMIC_DRAW);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, triangles_indirect_draw_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(DrawElementsIndirectCommand) * std::get<2>(max_mesh_count),
                     nullptr,
                     GL_DYNAMIC_DRAW);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, point_mesh_trsfm_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::mat4x4) * std::get<0>(max_mesh_count), nullptr,
                     GL_DYNAMIC_DRAW);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, line_mesh_trsfm_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::mat4x4) * std::get<1>(max_mesh_count), nullptr,
                     GL_DYNAMIC_DRAW);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, triangle_mesh_trsfm_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::mat4x4) * std::get<2>(max_mesh_count), nullptr,
                     GL_DYNAMIC_DRAW);

        draw_call_generator_program->use();
        draw_call_generator_program->set_uniform("material_idx", material_idx);

        // Execute the compute shader
        static constexpr unsigned GROUP_SIZE_X = 256; // Should be the same than GPU side
        glDispatchCompute((renderable_count + GROUP_SIZE_X - 1) / GROUP_SIZE_X, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_COMMAND_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, arb_counter_buffer);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint) * counter.size(), counter.data());

        for (int i = 0; i < 11; i++)
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, i, 0);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    RenderSystem::EnvironmentData
    RenderSystem::get_environment(World &world, const cpt::Camera3D &camera) {
        const EntityID camera_entity = camera.get_entity();

        const auto env_cpnt = world.get_component<cpt::Environment3D>(camera_entity);
        return EnvironmentData{
            env_cpnt->get_ambiant_light(), env_cpnt->get_cubemap(),
            env_cpnt->get_vao()
        };
    }

    void RenderSystem::draw_skybox(const rsc::CubemapTexture &cubemap, const cpt::Camera3D &camera,
                                   const GLuint vao) const {
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
}
