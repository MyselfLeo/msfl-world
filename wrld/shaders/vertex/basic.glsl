#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;
layout (location = 3) in vec2 aTexCoords;

out vec3 frag_pos;
out vec3 frag_normal;
out vec4 frag_color;
out vec2 frag_texcoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    frag_pos = vec3(model * vec4(aPos, 1.0));
    frag_normal = mat3(transpose(inverse(model))) * aNormal;
    frag_color = vec4(aColor, 1.0);
    frag_texcoords = aTexCoords;

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
