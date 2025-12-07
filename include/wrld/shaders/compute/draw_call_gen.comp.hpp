//
// Created by leo on 12/7/25.
//

#pragma once

#include <string>

namespace wrld::shader::comp {
    /// This compute shader fills the indirect draw call buffers
    /// with the appropriate data based on visibility of each
    /// renderable and the currently checked material.
    /// We can't really do that once for each material as it would
    /// require a dynamic amount of buffers.
    /// Maybe if the shader is in itself recreated based on the amount
    /// of materials ?
    inline std::string DRAW_CALL_GEN = R"(
#version 460

/* ---------- Data structures & functions ------------ */
/*       See RenderSystem.hpp for informations.        */

// Axis-Aligned bounding box
struct AABoundingBox {
    vec4 lower; // We will consider .w to always be 1
    vec4 upper; // idem
};

struct StaticMeshData {
    uint primitive_type;    // 0 -> points, 1 -> lines, 4 -> triangles
    uint material_idx;
    uint vao_element_start;
    uint vao_element_count;
};

struct StaticModelData {
    AABoundingBox bounding_box;
    uint mesh_start;
    uint mesh_count;
};

// Specifies that the given model should be drawn with the given transform.
struct Renderable {
    mat4 model_matrix;
    uint model_idx;
};

// Struct for MultiDrawElementsIndirect
struct ElementsParam
{
    uint index_count;     // Equivalent of vao_element_start::vao_element_count
    uint instance_count;  // For now always 1
    uint first_index;     // Equivalent of StaticMeshData::vao_element_start
    uint vertex_base;     // don't reindex vertices so 0
    uint instance_base;   // 1 instance so 0
};

/* ---------- IO ------------ */

// Input data
layout (std430, binding = 0) readonly buffer model_data_buffer {
    StaticModelData model_data[];
};
layout (std430, binding = 1) readonly buffer mesh_data_buffer {
    StaticMeshData mesh_data[];
};
layout (std430, binding = 2) readonly buffer renderable_buffer {
    Renderable renderables[];
};
layout (std430, binding = 3) readonly buffer visibility_buffer {
    bool visibility[];
};

// Output data
layout (std430, binding = 4) buffer arb_counter_buffer {
    uint point_count;
    uint line_count;
    uint triangle_count;
};
layout (std430, binding = 5) writeonly buffer points_indirect_draw_buffer {
    ElementsParam points_drawcall[];
};
layout (std430, binding = 6) writeonly buffer lines_indirect_draw_buffer {
    ElementsParam lines_drawcall[];
};
layout (std430, binding = 7) writeonly buffer triangles_indirect_draw_buffer {
    ElementsParam triangles_drawcall[];
};
layout (std430, binding = 8) writeonly buffer point_mesh_trsfm_buffer {
    mat4 point_mesh_trsfm[];
};
layout (std430, binding = 9) writeonly buffer line_mesh_trsfm_buffer {
    mat4 line_mesh_trsfm[];
};
layout (std430, binding = 10) writeonly buffer triangle_mesh_trsfm_buffer {
    mat4 triangle_mesh_trsfm[];
};

// This compute shader is called once per material
uniform uint material_idx;

/* ---------- Main function ------------ */

layout (local_size_x = 256) in; // Test up to 256 objects at once
void main() {
    uint id = gl_GlobalInvocationID.x;

    // Execute on every visible renderable
    if (id < renderables.length() && visibility[id]) {
        StaticModelData model = model_data[renderables[id].model_idx];
        mat4 transform = renderables[id].model_matrix;

        // Populate the indirect draw call buffers and mesh_trsfm buffers
        for (uint i = 0; i < model.mesh_count; i++) {
            StaticMeshData mesh = mesh_data[model.mesh_start + i];

            // Only fill the buffers with the meshes that have the correct
            // material. We'll render them, then call this compute shader
            // with another material, then again etc. until all materials
            // are rendered.
            if (mesh.material_idx != material_idx) continue;

            switch (mesh.primitive_type) {
                case 0:
                    {
                        // todo: don't do so much atomic adds
                        uint index = atomicAdd(point_count, 1u);

                        points_drawcall[index].index_count = mesh.vao_element_count;
                        points_drawcall[index].instance_count = 1;
                        points_drawcall[index].first_index = mesh.vao_element_start;
                        points_drawcall[index].vertex_base = 0;
                        points_drawcall[index].instance_base = 0;

                        point_mesh_trsfm[index] = transform;
                    } break;

                case 1:
                    {
                        // todo: don't do so much atomic adds
                        uint index = atomicAdd(line_count, 1u);

                        lines_drawcall[index].index_count = mesh.vao_element_count;
                        lines_drawcall[index].instance_count = 1;
                        lines_drawcall[index].first_index = mesh.vao_element_start;
                        lines_drawcall[index].vertex_base = 0;
                        lines_drawcall[index].instance_base = 0;

                        line_mesh_trsfm[index] = transform;
                    } break;

                case 4:
                    {
                        // todo: don't do so much atomic adds
                        uint index = atomicAdd(triangle_count, 1u);

                        triangles_drawcall[index].index_count = mesh.vao_element_count;
                        triangles_drawcall[index].instance_count = 1;
                        triangles_drawcall[index].first_index = mesh.vao_element_start;
                        triangles_drawcall[index].vertex_base = 0;
                        triangles_drawcall[index].instance_base = 0;

                        triangle_mesh_trsfm[index] = transform;
                    } break;
            }
        }
    }
}
)";
}
