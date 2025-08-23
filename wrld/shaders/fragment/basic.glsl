#version 460 core

#define MAX_LIGHTS 10

struct AmbiantLight {
    vec4 color;
};

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
};

struct DirectionalLight {
    vec3 direction;
    vec3 color;
};


in vec3 frag_pos;
in vec3 frag_normal;
in vec4 frag_color;
in vec2 frag_texcoords;

uniform uint point_light_nb;
uniform PointLight point_lights[MAX_LIGHTS];

uniform uint directional_lights_nb;
uniform PointLight directional_lights[MAX_LIGHTS];

uniform AmbiantLight ambiant_light;

uniform bool use_texture;
uniform sampler2D material_texture_diffuse0;
uniform sampler2D material_texture_specular0;

out vec4 FragColor;

/// Compute the color contribution of the given PointLight for the fragment
vec3 calc_point_light(PointLight pl) {

    // For now we only compute the diffuse component
    vec3 normal = normalize(frag_normal);
    float distance = length(pl.position - frag_pos);
    vec3 light_direction = normalize(pl.position - frag_pos);
    float angle = max(dot(normal, light_direction), 0.0);
    float attenuation = pl.intensity / (pl.intensity + distance * distance);
    vec3 diffuse = angle * pl.color * attenuation;

    return diffuse;
}


void main()
{
    vec3 light_res = ambiant_light.color.rgb * ambiant_light.color.a;

    for (int i = 0; i < point_light_nb; i++) {
        light_res += calc_point_light(point_lights[i]);
    }

    // If the diffuse texture is not specified, we use vertex color instead
    if (use_texture) {
        FragColor = texture(material_texture_diffuse0, frag_texcoords) * vec4(light_res, 1.0);
    } else {
        FragColor = frag_color * vec4(light_res, 1.0);
    }
}