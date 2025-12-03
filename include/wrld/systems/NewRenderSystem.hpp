//
// Created by leo on 11/28/25.
//

#pragma once
#include <set>

#include "glad/glad.h"
#include "glm/mat4x4.hpp"

#include <wrld/resources/Rc.hpp>
#include <wrld/resources/Material.hpp>

#include "wrld/components/Camera3D.hpp"
#include "wrld/resources/Model.hpp"

namespace wrld {

    /// This system manages :
    /// - Allocation of geometry to the GPU
    /// - Rendering pipeline (GPU culling -> Vertex shader -> Fragment shader)
    class NewRenderSystem {
    public:
        /// Initialize the system, compiles required shaders...
        static void init(World& world);

        /// Reload loaded meshes, models & materials. All the data
        /// is sent to the GPU.
        static void reload_resources(World& world);

        /// Render all the cameras in the world.
        static void render();

        /// Render the given camera.
        static void render_camera(World& world, const cpt::Camera3D &camera);

    private:
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
            GLint diffuse_map;           // Value not relevant if use_diffuse_map is false
            GLint specular_map;          // Idem
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
            GLuint material_idx;        // Material to use for this ARB
            GLuint draw_command_idx;    // We can retrieve the model_matrix from draw_command_buffer
        };

        /// Fill the draw buffers & arb data buffers with visibility information
        /// for the given camera.
        static void compute_draw_commands(World& world, const cpt::Camera3D &camera);

        /// Updates already loaded materials.
        /// Materiald are updated each time on the GPU. This may not be the best solution
        /// and I might do StaticMaterial & DynamicMaterial.
        static void reload_materials(World& world);

        /// Compute shader that fills draw buffers & arb data buffers based
        /// on the visiblity of models passed using DrawCommand.
        static Rc<rsc::Program> visibility_program;

        /// Tells for each rsc::Model its position in StaticModelData.
        static std::unordered_map<Rc<rsc::Model>, GLuint> model_indices;

        static std::vector<Rc<rsc::Material>> materials;

        // Textures used in the models.
        // We are sadly limited to 32 active textures (for now ? Maybe if we split draw calls...).
        // Before the draw call we have to do textures[i]->use(i).
        // Why : While we can give the materials to the fragment shader in storage buffers, we
        //       can't give textures as buffers. We can set 32 active textures and we have to do
        //       that manually before the draw calls (as they might have changed).
        static std::vector<Rc<rsc::Texture>> textures;

        static GLuint static_models_vao; // All static models are rendered using 1 draw call (hopefully)
        static GLuint static_models_vbo;
        static GLuint static_models_ebo;

        static GLuint model_data_buffer;
        static GLuint mesh_data_buffer;
        static GLuint draw_command_buffer;

        static GLuint materials_buffer;

        // todo: maybe use a vector instead
        static GLuint points_indirect_draw_buffer;
        static GLuint lines_indirect_draw_buffer;
        static GLuint triangles_indirect_draw_buffer;

        static GLuint arb_counter_buffer;

        static GLuint points_arb_data_buffer;
        static GLuint lines_arb_data_buffer;
        static GLuint triangles_arb_data_buffer;
    };
} // wrld