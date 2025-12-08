//
// Created by leo on 9/21/25.
//

#pragma once

#include <string>

namespace wrld::shader {
    inline std::string SKYBOX = R"(
#version 460 core

#ifdef VERTEX_SHADER

void main()
{
    // Triangle covering the whole screen
    vec2 vertices[3]= vec2[3](vec2(-1, -1), vec2(3, -1), vec2(-1, 3));
    gl_Position = vec4(vertices[gl_VertexID], 1.0, 1.0);
}

#endif

#ifdef FRAGMENT_SHADER

// From viewport to world (viewport -> projection -> view)
uniform mat4 inv_matrix;
uniform vec3 camera_pos;
uniform samplerCube cubemap;

out vec4 FragColor;

void main()
{
    vec4 p = inv_matrix * vec4(gl_FragCoord.xyz, 1);
    vec3 pixel_pos = p.xyz / p.w;

    vec3 direction = normalize(pixel_pos - camera_pos);

    FragColor = texture(cubemap, direction);
}

#endif
)";

}
