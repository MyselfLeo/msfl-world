#version 460 core

#ifdef VERTEX_SHADER

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 tex_coords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    tex_coords = aTexCoords;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}

#endif

#ifdef FRAGMENT_SHADER

out vec4 FragColor;

in vec2 tex_coords;

uniform sampler2D material_texture_diffuse0;
uniform sampler2D material_texture_specular0;

void main()
{
    FragColor = texture(material_texture_diffuse0, tex_coords);
    // FragColor = vec4(0.0, 0.0, 1.0, 1.0);
}

#endif