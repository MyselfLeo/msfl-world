//
// Created by leo on 9/21/25.
//

#ifndef DEFERRED_PASS1_SHADER_HPP
#define DEFERRED_PASS1_SHADER_HPP
#include <string>

namespace wrld::shader {
    inline std::string DEFERRED_PASS1 = R"(
#version 460 core

struct Material {
    vec4 diffuse_color;
    float specular_intensity;

    bool use_diffuse;// Equivalent to std::optional
    sampler2D diffuse;

    bool use_specular;
    sampler2D specular;

    float shininess;

    bool use_mesh_color;
    bool do_lighting;
};

// Output of Vertex Shader
in vec3 frag_pos;
in vec3 frag_normal;
in vec4 frag_color;
in vec2 frag_texcoords;

// Mesh material
uniform Material material;

out vec3 out_frag_pos;
out vec3 out_frag_normal;
out vec4 out_color;// diffuse (vec3) + specular (a)


vec4 sample_diffuse() {
    if (material.use_diffuse) {
        return texture(material.diffuse, frag_texcoords);
    }
    else if (material.use_mesh_color) {
        return frag_color;
    }
    else {
        return material.diffuse_color;
    }
}

float sample_specular() {
    if (material.use_specular) {
        return texture(material.specular, frag_texcoords).r;
    }
    else {
        return material.specular_intensity;
    }
}

void main() {
    if (sample_diffuse().a < 0.01) discard;

    out_frag_pos = frag_pos;
    out_frag_normal = frag_normal;
    // Why 1 - specular :
    // The texture will initially be (0, 0, 0, 1).
    // Thus 1 is the "default specular", 0. So we inverse the specular
    // in the texture to be sure that the sky (for example) is not lit up by
    // specular light.
    out_color = vec4(sample_diffuse().rgb, 1 - sample_specular());
}
)";

}

#endif // DEFERRED_PASS1_SHADER_HPP
