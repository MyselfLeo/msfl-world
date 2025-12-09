//
// Created by leo on 12/7/25.
//

#pragma once

#include <string>

namespace wrld::shader::frag {
    /// Default fragment shader to be used with a GPU-driven
    /// frustum culling.
    inline std::string DEFERRED_FIRST_PASS = R"(
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

// Output to the Framebuffer
out vec3 out_frag_pos;
out vec3 out_frag_normal;
out vec4 out_color; // diffuse (vec3) + specular (a)
out uint out_do_lighting;

/* ---------- Function ------------ */

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
    if (sample_diffuse().a < 0.01) discard;

    out_frag_pos = frag_pos;
    out_frag_normal = frag_normal;

    if (material.do_lighting) {
        out_do_lighting = 1;
    }
    else {
        out_do_lighting = 0;
    }

    // Why 1 - specular :
    // The texture will initially be (0, 0, 0, 1).
    // Thus 1 is the "default specular", 0. So we inverse the specular
    // in the texture to be sure that the sky (for example) is not lit up by
    // specular light.
    out_color = vec4(sample_diffuse().rgb, 1 - sample_specular());
}
)";
}
