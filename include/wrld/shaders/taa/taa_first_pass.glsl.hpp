//
// Created by leo on 1/26/26.
//

#pragma once

#include <string>

namespace wrld::shader {
    inline std::string TAA_FIRST_PASS = R"(
#version 460 core

#ifdef VERTEX_SHADER

#version 460 core
#extension GL_ARB_shader_draw_parameters: require

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

#endif

#ifdef FRAGMENT_SHADER

#version 460 core

/* ---------- Data structures  ------------ */
/*  See RenderSystem.hpp for informations.  */

// Specifies that the given model should be drawn with the given transform.
struct Renderable {
    mat4 model_matrix;
    uint model_idx;
};

struct Material {
    vec4 diffuse_color;
    float specular_intensity;
    bool use_diffuse_map;
    bool use_specular_map;
    sampler2D diffuse_map;
    sampler2D specular_map;
    uint shininess;
    bool use_vertex_color;
    bool do_lighting;
};

/* ---------- IO ------------ */

// Output of Vertex Shader
in vec3 frag_pos;
in vec3 frag_normal;
in vec4 frag_color;
in vec2 frag_texcoords;

// This shader is called once per material
uniform Material material;

uniform float elapsed_time;
uniform vec2 window_size;

// Output to the Framebuffer
out vec3 out_frag_pos;
out vec3 out_frag_normal;
out vec4 out_color; // diffuse (vec3) + specular (a)
out uint out_do_lighting;

/* ---------- Function ------------ */

// Hash x (x = seed, returns a random number)
// x must be odd.
uint hash(uint x)
{
    x ^= x >> 16;
    x *= 0x21f0aaad;
    x ^= x >> 15;
    x *= 0xd35a2d97;
    x ^= x >> 15;
    return x;
}

// Returns a random number between 0 and 1.
float random(uint seed) {
    seed = (seed << 1) | uint(1);
    return float(hash(seed)) / 4294967296.0;
}

uint elapsed_ms() {
    return uint(elapsed_time * 1000);
}


// Return, based on the material, the diffuse color of this fragment.
vec4 sample_diffuse() {
    if (material.use_diffuse_map) {
        return texture(material.diffuse_map, frag_texcoords);
    }
    else if (material.use_vertex_color) {
        return frag_color;
    }
    else {
        return material.diffuse_color;
    }
}

// Return, based on the material, the specular intensity of this fragment.
float sample_specular() {
    if (material.use_specular_map) {
        return float(texture(material.specular_map, frag_texcoords));
    }
    else {
        return material.specular_intensity;
    }
}

/* ---------- Main function ------------ */

void main() {
    uint idx = uint(gl_FragCoord.y) * uint(window_size.x) + uint(gl_FragCoord.x);
    uint depth = uint(gl_FragCoord.z * 4294967296.0);
    float rdm = random(elapsed_ms() * idx * depth);

    if (rdm > sample_diffuse().a) discard;

    out_frag_pos = frag_pos;
    out_frag_normal = frag_normal;

    if (material.do_lighting) {
        out_do_lighting = 1;
    }
    else {
        out_do_lighting = 0;
    }

    out_color = vec4(sample_diffuse().rgb, sample_specular());
}

#endif
)";
}
