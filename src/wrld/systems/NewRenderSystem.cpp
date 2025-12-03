//
// Created by leo on 11/28/25.
//

#include <wrld/systems/NewRenderSystem.hpp>

#include "wrld/Main.hpp"
#include "wrld/World.hpp"
#include "wrld/components/StaticModel.hpp"

namespace wrld {
    GLuint NewRenderSystem::static_models_vao = 0;
    GLuint NewRenderSystem::static_models_vbo = 0;
    GLuint NewRenderSystem::static_models_ebo = 0;
    GLuint NewRenderSystem::model_data_buffer = 0;
    GLuint NewRenderSystem::mesh_data_buffer = 0;
    GLuint NewRenderSystem::draw_command_buffer = 0;
    GLuint NewRenderSystem::materials_buffer = 0;
    GLuint NewRenderSystem::points_indirect_draw_buffer = 0;
    GLuint NewRenderSystem::lines_indirect_draw_buffer = 0;
    GLuint NewRenderSystem::triangles_indirect_draw_buffer = 0;
    GLuint NewRenderSystem::arb_counter_buffer = 0;
    GLuint NewRenderSystem::points_arb_data_buffer = 0;
    GLuint NewRenderSystem::lines_arb_data_buffer = 0;
    GLuint NewRenderSystem::triangles_arb_data_buffer = 0;

    std::vector<Rc<rsc::Material>> NewRenderSystem::materials;
    std::vector<Rc<rsc::Texture>> NewRenderSystem::textures;
    std::unordered_map<Rc<rsc::Model>, GLuint> NewRenderSystem::model_indices;
    Rc<rsc::Program> NewRenderSystem::visibility_program = {};


    void NewRenderSystem::init(World &world) {
        visibility_program = world.create_resource<rsc::Program>("Visiblity Compute Shader");
        // todo: put this compute in a .hpp
        visibility_program->shader_path(rsc::ShaderType::Compute, "/mnt/Projects/mif-si3D/rungholt/data/compute/frustum_culling_v2.comp");

        // ARB counter buffer, 3 uint (point, line, triangle)
        glGenBuffers(1, &arb_counter_buffer);
    }

    void NewRenderSystem::reload_resources(World &world) {
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
            const auto& model = raw_model.as<rsc::Model>();
            for (const auto& [mg, _]: model->get_mesh_groups()) {
                for (const auto& mesh: mg.get_meshes()) {
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
            const auto& model = raw_model.as<rsc::Model>();

            const GLuint mesh_start = mesh_data.size();
            GLuint mesh_count = 0;

            for (const auto& [mg, mat_id]: model->get_mesh_groups()) {
                // Material & textures
                const auto& mat = model->get_materials()[mat_id];
                unsigned global_material_index;
                if (auto i = std::ranges::find(materials, mat); i != materials.end()) {
                    global_material_index = i - materials.begin();
                } else {
                    global_material_index = materials.size();
                    materials.push_back(mat);
                }

                // Mesh data
                for (const auto& mesh: mg.get_meshes()) {
                    mesh_count += 1;

                    const GLuint vao_start = elements.size();
                    const GLuint vao_count = mesh.get_element_count();

                    // Those functions exist despite CLion telling they don't
                    vertices.insert(vertices.end(), mesh.get_vertices().begin(), mesh.get_vertices().end());
                    elements.insert(elements.end(), mesh.get_elements().begin(), mesh.get_elements().end());

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

        Main::set_statistic("StaticModelData count", std::to_string(model_data.size()));
        Main::set_statistic("StaticMeshData count", std::to_string(mesh_data.size()));

        // Send data to the GPU.
        // First we send the geometry data (for vertex & fragment shaders),
        // then the information required by the compute shader,
        // then the materials.

        // If the buffers were not created yet
        if (static_models_vao == 0)
            glGenVertexArrays(1, &static_models_vao);
        if (static_models_vbo == 0)
            glGenBuffers(1, &static_models_vbo);
        if (static_models_ebo == 0)
            glGenBuffers(1, &static_models_ebo);
        if (model_data_buffer == 0)
            glGenBuffers(1, &model_data_buffer);
        if (mesh_data_buffer == 0)
            glGenBuffers(1, &mesh_data_buffer);
        if (draw_command_buffer == 0)
            glGenBuffers(1, &draw_command_buffer);
        if (points_indirect_draw_buffer == 0)
            glGenBuffers(1, &points_indirect_draw_buffer);
        if (lines_indirect_draw_buffer == 0)
            glGenBuffers(1, &lines_indirect_draw_buffer);
        if (triangles_indirect_draw_buffer == 0)
            glGenBuffers(1, &triangles_indirect_draw_buffer);
        if (points_arb_data_buffer == 0)
            glGenBuffers(1, &points_arb_data_buffer);
        if (lines_arb_data_buffer == 0)
            glGenBuffers(1, &lines_arb_data_buffer);
        if (triangles_arb_data_buffer == 0)
            glGenBuffers(1, &triangles_arb_data_buffer);


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

        ///////////////// Compute shader information

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, model_data_buffer);
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(StaticModelData) * model_data.size(), model_data.data(), 0);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, mesh_data_buffer);
        glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(StaticMeshData) * mesh_data.size(), mesh_data.data(), 0);

        reload_materials(world);
    }

    void NewRenderSystem::render() {
    }

    void NewRenderSystem::render_camera(World &world, const cpt::Camera3D &camera) {
        compute_draw_commands(world, camera);
    }

    void NewRenderSystem::compute_draw_commands(World &world, const cpt::Camera3D &camera) {
        reload_materials(world);

        // Find all entities that should be rendered
        const std::vector<EntityID> renderables = world.get_entities_with_component<cpt::StaticModel>();

        Main::set_statistic("Renderables", std::to_string(renderables.size()));

        std::vector<DrawCommand> commands;
        commands.reserve(renderables.size());

        unsigned max_point_mesh = 0;
        unsigned max_line_mesh = 0;
        unsigned max_triangle_mesh = 0;

        for (const auto entity: renderables) {
            const auto transform_cpt = world.get_component<cpt::Transform>(entity);
            const auto model_cpt = world.get_component<cpt::StaticModel>(entity);
            const auto model = model_cpt->get_model();

            for (const auto& mg: model->get_mesh_groups() | std::views::keys) {
                switch (mg.get_primitive_type()) {
                    case obj::PrimitiveType::Points:
                        max_point_mesh += mg.get_mesh_count();
                        break;
                    case obj::PrimitiveType::Lines:
                        max_line_mesh += mg.get_mesh_count();
                        break;
                    case obj::PrimitiveType::Triangles:
                        max_triangle_mesh += mg.get_mesh_count();
                        break;
                }
            }

            commands.emplace_back(transform_cpt->model_matrix(), model_indices[model]);
        }

        Main::set_statistic("Max point meshes", std::to_string(max_point_mesh));
        Main::set_statistic("Max line meshes", std::to_string(max_line_mesh));
        Main::set_statistic("Max triangle meshes", std::to_string(max_triangle_mesh));

        // Send data to GPU
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, draw_command_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(DrawCommand) * commands.size(), commands.data(), GL_DYNAMIC_DRAW);

        // Allocate memory for the draw_buffers & the arb_data_buffers based on
        // the max count computed at the previous step.
        static constexpr unsigned ElementsParamSize = 6 * sizeof(GLuint);
        static constexpr unsigned ARBDataSize = 2 * sizeof(GLuint);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, points_indirect_draw_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, ElementsParamSize * max_point_mesh, nullptr, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, lines_indirect_draw_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, ElementsParamSize * max_line_mesh, nullptr, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, triangles_indirect_draw_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, ElementsParamSize * max_triangle_mesh, nullptr, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, points_arb_data_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, ARBDataSize * max_point_mesh, nullptr, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, lines_arb_data_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, ARBDataSize * max_line_mesh, nullptr, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, triangles_arb_data_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, ARBDataSize * max_triangle_mesh, nullptr, GL_DYNAMIC_DRAW);

        // Prepare the dispatch
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, model_data_buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mesh_data_buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, draw_command_buffer);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, points_indirect_draw_buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, lines_indirect_draw_buffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, triangles_indirect_draw_buffer);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, arb_counter_buffer);


        std::array<GLuint, 3> counter{0, 0, 0};
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, arb_counter_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint) * 3, counter.data(), GL_DYNAMIC_DRAW);

        visibility_program->use();
        visibility_program->set_uniform("view_matrix", camera.get_view_matrix());
        visibility_program->set_uniform("proj_matrix", camera.get_projection_matrix());

        // Execute the compute shader
        static constexpr unsigned GROUP_SIZE_X = 256; // Should be the same than GPU side
        glDispatchCompute((renderables.size() + GROUP_SIZE_X - 1) / GROUP_SIZE_X, 1, 1);
        glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

        glBindBuffer(GL_PARAMETER_BUFFER_ARB, arb_counter_buffer);
        glGetBufferSubData(GL_PARAMETER_BUFFER_ARB, 0, sizeof(GLuint) * 3, counter.data());

        Main::set_statistic("ARB Frustum POINT count", std::to_string(counter[0]));
        Main::set_statistic("ARB Frustum LINE count", std::to_string(counter[1]));
        Main::set_statistic("ARB Frustum TRIANGLE count", std::to_string(counter[2]));
    }

    void NewRenderSystem::reload_materials(World &world) {
        textures.clear();

        std::vector<MaterialData> material_data;

        for (const auto& mat: materials) {
            // Check if the texture is already listed as "to be active", if yes then
            // just give its attributed active position, else add it if there is still
            // space for it. If not we raise a warning and just give it texture 0.
            GLint diffuse_map = 0, specular_map = 0;
            if (mat->get_diffuse_map().has_value()) {
                const auto texture = mat->get_diffuse_map().value();
                if (auto j = std::ranges::find(textures, texture); j != textures.end()) {
                    diffuse_map = j - textures.begin();
                } else {
                    if (textures.size() == 32) {
                        wrldInfo("Failed to load more than 32 active textures. Sorry !");
                    } else {
                        diffuse_map = textures.size();
                        textures.push_back(texture);
                    }
                }
            }
            if (mat->get_specular_map().has_value()) {
                const auto texture = mat->get_specular_map().value();
                if (auto j = std::ranges::find(textures, texture); j != textures.end()) {
                    specular_map = j - textures.begin();
                } else {
                    if (textures.size() == 32) {
                        wrldInfo("Failed to load more than 32 active textures. Sorry !");
                    } else {
                        specular_map = textures.size();
                        textures.push_back(texture);
                    }
                }
            }

            material_data.emplace_back(
                mat->get_diffuse_color(),
                mat->get_specular_intensity(),
                mat->get_diffuse_map().has_value(),
                mat->get_specular_map().has_value(),
                diffuse_map,
                specular_map,
                mat->get_shininess(),
                mat->is_using_mesh_color(),
                mat->is_doing_lighting()
                );
        }

        ///////////////// Materials

        if (materials_buffer == 0)
            glGenBuffers(1, &materials_buffer);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, materials_buffer);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(MaterialData) * material_data.size(), material_data.data(), GL_DYNAMIC_DRAW);
    }
} // wrld