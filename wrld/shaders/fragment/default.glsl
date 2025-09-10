#version 460 core

// Max light per each type
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

// Position of the camera in world space
uniform vec3 view_pos;

// Light data in the world
uniform AmbiantLight ambiant_light;

uniform uint point_light_nb;
uniform PointLight point_lights[MAX_LIGHTS];

uniform uint directional_lights_nb;
uniform DirectionalLight directional_lights[MAX_LIGHTS];

// Mesh material
uniform Material material;

out vec4 FragColor;

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
        return float(texture(material.specular, frag_texcoords));
    }
    else {
        return material.specular_intensity;
    }
}

float calc_diffuse(vec3 light_direction, vec3 normal) {
    return max(dot(normal, light_direction), 0.0);
}

float calc_specular(vec3 light_direction, vec3 view_direction, vec3 normal) {
    vec3 halfway_direction = normalize(light_direction + view_direction);
    return pow(max(dot(normal, halfway_direction), 0.0), material.shininess);
}

vec4 calc_ambiant_light() {
    return vec4(ambiant_light.color, 1.0) * sample_diffuse() * ambiant_light.intensity;
}

/// Compute the color contribution of the given PointLight for the fragment
vec4 calc_point_light(PointLight pl) {
    // Attenuation due to the distance from the point light
    // todo: Maybe use the sphere area equation
    float distance = length(pl.position - frag_pos);

    float attenuation = 0.0;
    if (pl.intensity > 0) {
        attenuation = pl.intensity / (pl.intensity + distance * distance);
    }

    vec3 light_direction = normalize(pl.position - frag_pos);
    vec3 view_direction = normalize(view_pos - frag_pos);
    vec3 normal = normalize(frag_normal);

    float diffuse_amount = calc_diffuse(light_direction, normal);
    float specular_amount = calc_specular(light_direction, view_direction, normal);

    vec4 diffuse = vec4(pl.color, 1.0) * sample_diffuse();
    vec3 specular = pl.color * sample_specular();

    // todo : specular is probably not correct for transparent values
    return (diffuse * diffuse_amount + vec4(specular * specular_amount, sample_specular())) * pl.intensity * attenuation;
}

/// Compute the color contribution of the given DirectionalLight for the fragment
vec4 calc_directional_light(DirectionalLight dl) {
    vec3 light_direction = normalize(-dl.direction);
    vec3 view_direction = normalize(view_pos - frag_pos);
    vec3 normal = normalize(frag_normal);

    float diffuse_amount = calc_diffuse(light_direction, normal);
    float specular_amount = calc_specular(light_direction, view_direction, normal);

    vec4 diffuse = vec4(dl.color, 1.0) * sample_diffuse();
    vec3 specular = dl.color * sample_specular();

    return (diffuse * diffuse_amount + vec4(specular * specular_amount, sample_specular())) * dl.intensity;
}


void main() {
    if (!material.do_lighting) {
        FragColor = sample_diffuse();
        return;
    }

    vec4 res = vec4(0.0);

    // Process ambient light
    res += calc_ambiant_light();

    // Process point lights
    for (int i = 0; i < point_light_nb; i++) {
        res += calc_point_light(point_lights[i]);
    }

    // Process directional lights
    for (int i = 0; i < directional_lights_nb; i++) {
        res += calc_directional_light(directional_lights[i]);
    }

    FragColor = res;
}