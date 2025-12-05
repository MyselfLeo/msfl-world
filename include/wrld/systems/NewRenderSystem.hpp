//
// Created by leo on 11/28/25.
//

#pragma once

#include "glad/glad.h"
#include "glm/mat4x4.hpp"

#include <wrld/resources/Rc.hpp>
#include <wrld/resources/Material.hpp>

#include "wrld/components/Camera3D.hpp"
#include "wrld/components/Environment3D.hpp"
#include "wrld/resources/Model.hpp"

namespace wrld::sys {
    enum class RenderingType {
        Forward,
        Deferred,
    };

    /// This system manages :
    /// - Allocation of geometry to the GPU
    /// - Rendering pipeline (GPU culling -> Vertex shader -> Fragment shader)
    class NewRenderSystem {
    public:
        static NewRenderSystem *get();

        NewRenderSystem() = default;

        NewRenderSystem(NewRenderSystem &other) = delete;

        void operator=(const NewRenderSystem &) = delete;

        static constexpr unsigned MAX_LIGHTS = 100;

        /// Initialize the system, compiles required shaders...
        void init(World &world);

        /// Reload loaded meshes, models & materials. All the data
        /// is sent to the GPU.
        void reload_resources(World &world);

        /// Render all the cameras in the world.
        void render(World &world);

    private:
        // ------------------ Singleton pattern ------------------ //
        static NewRenderSystem *singleton;

        // ------------------ Private data structures ------------------ //

        /// For each mesh we give :
        /// - Its primitive type. The compute shader will use it
        ///   to put the draw command in the correct Indirect Buffer.
        /// - The material it uses
        /// - Its position in the VAO
        struct alignas(16) StaticMeshData {
            GLenum primitive_type;
            GLuint material_idx;
            GLuint vao_element_start;
            GLuint vao_element_count;
        };

        struct alignas(16) AABB {
            glm::vec4 lower;
            glm::vec4 upper;
        };

        /// For each mesh we give :
        /// - Its bounding box (lower & upper bounds)
        /// - The first mesh it uses in the global
        /// - Its mesh count
        struct alignas(16) StaticModelData {
            AABB aabb;
            GLuint mesh_start = 0;
            GLuint mesh_count = 0;
        };

        // Fixme: It might be a good idea to reduce the size
        // of this struct.
        // Some possibilities :
        // - Combine all booleans into 1 GLuint and use bitwise
        //   operations
        // - If diffuse_color.w is not used, store the specular_intensity
        //   in it instead.
        // - diffuse_map and specular_map, if used, will have values between 0 and 31,
        //   so we can use a special value (-1, 100, 42, idc) to denote "not used".
        struct alignas(16) MaterialData {
            glm::vec4 diffuse_color;
            GLfloat specular_intensity;
            GLboolean use_diffuse_map;
            GLboolean use_specular_map;
            GLint diffuse_map;  // Value not relevant if use_diffuse_map is false
            GLint specular_map; // Idem
            GLuint shininess;
            GLboolean use_vertex_color;
            GLboolean do_lighting;
        };

        /// Tells the compute shader that the model_idx should be rendered
        /// using the transform matrix.
        struct alignas(16) DrawCommand {
            glm::mat4x4 transform;
            GLuint model_idx;
        };

        /// The vertex/fragment shader will be called with a different ARB index
        /// for each mesh. This tells the pipeline how each mesh is rendered.
        struct ARBData {
            GLuint material_idx;     // Material to use for this ARB
            GLuint draw_command_idx; // We can retrieve the model_matrix from draw_command_buffer
        };

        struct alignas(16) DrawArraysIndirectCommand {
            GLuint index_count;
            GLuint instance_count;
            GLuint first_index;
            GLuint vertex_base;
            GLuint instance_base;
        };

        struct PointLightData {
            glm::vec3 position;
            glm::vec3 color;
            float intensity;
        };

        struct DirectionalLightData {
            glm::vec3 direction;
            glm::vec3 color;
            float intensity;
        };

        struct EnvironmentData {
            cpt::AmbiantLight ambiant_light;
            std::optional<Rc<rsc::CubemapTexture> > skybox;
            GLuint vao = 0;
        };

        struct LightCollection {
            std::vector<DirectionalLightData> directional_lights;
            std::vector<PointLightData> point_lights;
        };

        // ------------------ Private functions ------------------ //

        /// Render the given camera.
        void render_camera(World &world, const cpt::Camera3D &camera,
                           const LightCollection &lights);

        /// Fill the draw buffers & arb data buffers with visibility information
        /// for the given camera.
        /// Return the maximum draw count.
        GLuint compute_draw_commands(World &world, const cpt::Camera3D &camera);

        /// Updates already loaded materials.
        /// Materiald are updated each time on the GPU. This may not be the best solution
        /// and I might do StaticMaterial & DynamicMaterial.
        void reload_materials(World &world);

        /// Return the environment attached to the camera, or a default one if not
        /// provided.
        static EnvironmentData get_environment(World &world, const cpt::Camera3D &camera);

        /// Draw skybox onto the current framebuffer.
        void draw_skybox(const rsc::CubemapTexture &cubemap,
                         const cpt::Camera3D &camera, GLuint vao) const;

        /// Return data of all PointLights in the world.
        /// Won't return more than MAX_LIGHTS. The vector is resized before
        /// being returned.
        static std::vector<PointLightData> get_point_lights(World &world);

        /// Return data of all DirectionalLights in the world.
        /// Won't return more than MAX_LIGHTS. The vector is resized before
        /// being returned.
        static std::vector<DirectionalLightData> get_directional_lights(World &world);

        /// Add scene-related uniforms to the given program (Notably light data).
        static void set_scene_uniforms(const Rc<rsc::Program> &program,
                                       const cpt::AmbiantLight &ambiant_light, const LightCollection &lights);

        /// Add camera-related uniforms to the given program.
        static void set_camera_uniforms(const Rc<rsc::Program> &program, const cpt::Camera3D &camera);

        /// Bind to the given program the following buffers :
        /// - The ARB Data buffer related to the primitive type (binding 0)
        /// - The material buffer (binding 1)
        /// - The DrawCommands buffer (binding 2)
        void bind_uniform_buffers(obj::PrimitiveType primitive_type) const;

        // ------------------ Parameters ------------------ //

        RenderingType rendering_type = RenderingType::Forward;

        // ------------------ Resources ------------------ //

        /// Compute shader that fills draw buffers & arb data buffers based
        /// on the visiblity of models passed using DrawCommand.
        Rc<rsc::Program> visibility_program;

        /// Vertex + Fragment that draws the skybox.
        Rc<rsc::Program> skybox_program;

        /// Forward program.
        /// todo: Use the camera's program instead.
        Rc<rsc::Program> forward_program;

        /// First step of the deferred rendering process (Vertex + Fragment).
        /// This program will write relevant data into a DeferredFramebuffer.
        Rc<rsc::Program> deferred_first_pass;

        /// Second step of the deferred rendering process (Vertex + Fragment).
        /// This program takes the data from the previous Framebuffer and renders
        /// it correctly with all the materials applied.
        Rc<rsc::Program> deferred_second_pass;

        /// Tells for each rsc::Model its position in StaticModelData.
        std::unordered_map<Rc<rsc::Model>, GLuint> model_indices;

        std::vector<Rc<rsc::Material> > materials;

        // Textures used in the models.
        // We are sadly limited to 32 active textures (for now ? Maybe if we split draw calls...).
        // Before the draw call we have to do textures[i]->use(i).
        // Why : While we can give the materials to the fragment shader in storage buffers, we
        //       can't give textures as buffers. We can set 32 active textures and we have to do
        //       that manually before the draw calls (as they might have changed).
        std::vector<Rc<rsc::Texture> > textures;

        // ------------------ Buffers ------------------ //

        GLuint static_models_vao = 0; // All models are rendered using 1 draw call (hopefully)
        GLuint static_models_vbo = 0;
        GLuint static_models_ebo = 0;

        GLuint model_data_buffer = 0;
        GLuint mesh_data_buffer = 0;
        GLuint draw_command_buffer = 0;

        GLuint materials_buffer = 0;

        // todo: maybe use a vector instead
        GLuint points_indirect_draw_buffer = 0;
        GLuint lines_indirect_draw_buffer = 0;
        GLuint triangles_indirect_draw_buffer = 0;

        GLuint arb_counter_buffer = 0;

        GLuint points_arb_data_buffer = 0;
        GLuint lines_arb_data_buffer = 0;
        GLuint triangles_arb_data_buffer = 0;
    };
} // wrld
