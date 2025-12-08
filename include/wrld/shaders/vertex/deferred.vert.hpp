//
// Created by leo on 12/7/25.
//

#pragma once

#include <string>

namespace wrld::shader::vert {
    /// Default vertex transformation to be used with a GPU-driven
    /// frustum culling.
    inline std::string DEFERRED_FIRST_PASS = R"(
#version 460 core
#extension GL_ARB_shader_draw_parameters: require

/* ---------- Data structures & functions ------------ */
/*       See RenderSystem.hpp for informations.        */

// Specifies that the given model should be drawn with the given transform.
struct Renderable {
    mat4 model_matrix;
    uint model_idx;
};

/* ---------- IO ------------ */

// VAO data
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;
layout (location = 3) in vec2 aTexCoords;

layout (std430, binding = 0) readonly buffer mesh_trsfm_buffer {
    mat4 mesh_trsfm[];
};

uniform mat4 view;
uniform mat4 projection;

out vec3 frag_pos;
out vec3 frag_normal;
out vec4 frag_color;
out vec2 frag_texcoords;

/* ---------- Main function ------------ */

void main() {
    mat4 model_matrix = mesh_trsfm[gl_DrawID];
    mat4 model_normal = transpose(inverse(model_matrix));

    frag_pos = vec3(model_matrix * vec4(aPos, 1.0));
    frag_normal = vec3(model_normal * vec4(aNormal, 1.0));
    frag_color = vec4(aColor, 1.0);
    frag_texcoords = aTexCoords;

    gl_Position = projection * view * model_matrix * vec4(aPos, 1.0);
}
)";
}
