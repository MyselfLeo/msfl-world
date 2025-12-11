//
// Created by leo on 12/8/25.
//

#pragma once

#include "glad/glad.h"

#include <wrld/World.hpp>
#include <wrld/components/Environment3D.hpp>
#include <wrld/resources/Model.hpp>
#include <wrld/resources/Program.hpp>

namespace wrld::sys {
    class RenderSystem {
    public:
        static constexpr unsigned MAX_LIGHTS = 100;

        virtual ~RenderSystem() = default;

        /// Initialize the system, compiles required shaders...
        virtual void init(World &world);

        /// Render all the cameras in the world.
        virtual void render(World &world) = 0;

        /// Reload loaded meshes, models & materials. All the data
        /// is sent to the GPU.
        void reload_resources(const World &world);

    protected:
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

        /// Tells the compute shader that the model_idx should be rendered
        /// using the transform matrix.
        struct alignas(16) Renderable {
            glm::mat4x4 transform;
            GLuint model_idx;
        };

        /// The vertex/fragment shader will be called with a different ARB index
        /// for each mesh. This tells the pipeline how each mesh is rendered.
        struct ARBData {
            GLuint material_idx; // Material to use for this ARB
            GLuint draw_command_idx;
            // We can retrieve the model_matrix from draw_command_buffer
        };

        struct alignas(16) DrawElementsIndirectCommand {
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

        /// This function will :
        /// - Fill the "renderables" buffer with information about every renderable object
        /// - Fill the "visibility" buffer with a boolean for each renderable. This is done with
        ///   a compute shader.
        ///   Returns the max mesh count for each primitive type (point, line, triangle).
        void compute_renderables_visiblity(
                World &world, const cpt::Camera3D &camera);

        void compute_draw_calls_for_material(
                unsigned material_idx) const;

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
                                       const cpt::AmbiantLight &ambiant_light,
                                       const LightCollection &lights);

        /// Add camera-related uniforms to the given program.
        static void set_camera_uniforms(const Rc<rsc::Program> &program,
                                        const cpt::Camera3D &camera);

        void bind_trsfm_buffer(obj::PrimitiveType primitive_type) const;

        // ------------------ Resources ------------------ //

        /// This compute shader takes a list of renderables and fill the
        /// visiblity buffer with a boolean for each renderable telling if
        /// it is visible or not.
        Rc<rsc::Program> visibility_program;

        /// This compute shader takes the visibility data from visibility_program,
        /// a material index, and fills the ARB buffers with the correct information.
        Rc<rsc::Program> draw_call_generator_program;

        /// Vertex + Fragment that draws the skybox.
        Rc<rsc::Program> skybox_program;

        /// Tells for each rsc::Model its position in StaticModelData.
        std::unordered_map<Rc<rsc::Model>, GLuint> model_indices;

        std::vector<Rc<rsc::Material> > materials;

        // ------------------ Buffers ------------------ //

        GLuint static_models_vao = 0;
        // All models are rendered using 1 draw call (hopefully)
        GLuint static_models_vbo = 0;
        GLuint static_models_ebo = 0;

        GLuint model_data_buffer = 0;
        GLuint mesh_data_buffer = 0;

        GLuint renderable_buffer = 0;
        GLuint visibility_buffer = 0;
        // Tells for each draw command if it is visible or not

        // todo: maybe use a vector instead
        GLuint points_indirect_draw_buffer = 0;
        GLuint lines_indirect_draw_buffer = 0;
        GLuint triangles_indirect_draw_buffer = 0;

        GLuint arb_counter_buffer = 0;

        GLuint point_mesh_trsfm_buffer = 0;
        GLuint line_mesh_trsfm_buffer = 0;
        GLuint triangle_mesh_trsfm_buffer = 0;

        // ------------------ Misc. ------------------ //
        // Number of renderable objects for the current render iteration.
        unsigned renderable_count = 0;

        // Max number of mesh rendered at once for the current
        // render iteration.
        unsigned max_mesh_count = 0;
    };
}
