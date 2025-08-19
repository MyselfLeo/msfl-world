#version 460 core

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