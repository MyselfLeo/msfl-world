#version 460 core

out vec4 FragColor;

in vec3 frag_pos;
in vec3 frag_normal;
in vec4 frag_color;
in vec2 frag_texcoords;

uniform vec4 ambiant_color;
uniform float ambiant_strength;

uniform vec3 light_position;
uniform vec4 light_color;

uniform bool use_texture;
uniform sampler2D material_texture_diffuse0;
uniform sampler2D material_texture_specular0;

void main()
{
    // Ambiant component
    vec4 ambiant = ambiant_strength * ambiant_color;

    // Diffuse component
    vec3 norm = normalize(frag_normal);
    vec3 light_direction = normalize(light_position - frag_pos);
    float diff = max(dot(norm, light_direction), 0.0);
    vec4 diffuse = diff * light_color;

    vec4 light = ambiant + diffuse;

    // If the diffuse texture is not specified, we use vertex color instead
    if (use_texture) {
        FragColor = texture(material_texture_diffuse0, frag_texcoords) * light;
    } else {
        FragColor = frag_color * light;
    }
}