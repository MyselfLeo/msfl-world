#version 460 core

#ifdef VERTEX_SHADER

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;
layout (location = 3) in vec2 aTexCoords;

out vec4 color;
out vec2 tex_coords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    tex_coords = aTexCoords;
    color = vec4(aColor, 1.0);
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}

#endif

#ifdef FRAGMENT_SHADER

out vec4 FragColor;

in vec4 color;
in vec2 tex_coords;

uniform sampler2D material_texture_diffuse0;
uniform sampler2D material_texture_specular0;
uniform bool use_texture;

void main()
{
    // If the diffuse texture is not specified, we use vertex color instead
    if (use_texture) {
        FragColor = texture(material_texture_diffuse0, tex_coords);
    } else {
        FragColor = color;
    }
}

#endif