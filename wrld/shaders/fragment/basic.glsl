#version 460 core

#define MAX_LIGHTS 10

struct AmbiantLight {
    vec3 color;
    float intensity;
};

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
};

struct DirectionalLight {
    vec3 direction;
    vec3 color;
    float intensity;
};


in vec3 frag_pos;
in vec3 frag_normal;
in vec4 frag_color;
in vec2 frag_texcoords;

uniform vec3 view_pos;

uniform AmbiantLight ambiant_light;

uniform uint point_light_nb;
uniform PointLight point_lights[MAX_LIGHTS];

uniform uint directional_lights_nb;
uniform DirectionalLight directional_lights[MAX_LIGHTS];

uniform bool use_texture;
uniform sampler2D material_texture_diffuse0;
uniform sampler2D material_texture_specular0;

out vec4 FragColor;

/// Compute the color contribution of the given PointLight for the fragment
vec3 calc_point_light(PointLight pl) {
    float distance = length(pl.position - frag_pos);

    float attenuation = 0.0;
    if (pl.intensity > 0) {
        attenuation = pl.intensity / (pl.intensity + distance * distance);
    }

    vec3 normal = normalize(frag_normal);

    // Diffuse component
    vec3 light_direction = normalize(pl.position - frag_pos);
    float diff_amount = max(dot(normal, light_direction), 0.0);
    vec3 diffuse = diff_amount * pl.color * attenuation;

    // Specular component
    float specular_intensity = 0.5;// todo : change this to material parameters
    float shinyness = 32;// todo : change this to material parameters

    vec3 view_direction = normalize(view_pos - frag_pos);
    vec3 reflection_vector = reflect(-light_direction, normal);
    float spec_amount = pow(max(dot(view_direction, reflection_vector), 0.0), shinyness);
    vec3 specular = spec_amount * pl.color * specular_intensity * attenuation;

    return diffuse + specular;
}


/// Compute the color contribution of the given DirectionalLight for the fragment
vec3 calc_directional_light(DirectionalLight dl) {
    vec3 normal = normalize(frag_normal);

    // Diffuse component
    vec3 light_direction = normalize(-dl.direction);
    float diff_amount = max(dot(normal, light_direction), 0.0);
    vec3 diffuse = diff_amount * dl.color * dl.intensity;

    // Specular component
    float specular_intensity = 0.5;// todo : change this to material parameters
    float shinyness = 32;// todo : change this to material parameters

    vec3 view_direction = normalize(view_pos - frag_pos);
    vec3 reflection_vector = reflect(-light_direction, normal);
    float spec_amount = pow(max(dot(view_direction, reflection_vector), 0.0), shinyness);
    vec3 specular = spec_amount * dl.color * specular_intensity * dl.intensity;

    return diffuse + specular;
}


void main()
{
    vec3 light_res = ambiant_light.color * ambiant_light.intensity;

    for (int i = 0; i < point_light_nb; i++) {
        light_res += calc_point_light(point_lights[i]);
    }

    for (int i = 0; i < directional_lights_nb; i++) {
        light_res += calc_directional_light(directional_lights[i]);
    }

    // If the diffuse texture is not specified, we use vertex color instead
    if (use_texture) {
        FragColor = texture(material_texture_diffuse0, frag_texcoords) * vec4(light_res, 1.0);
    } else {
        FragColor = frag_color * vec4(light_res, 1.0);
    }
}