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

// Max light per each type
#define MAX_LIGHTS 100

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

// From viewport to world (viewport -> projection -> view)
//uniform sampler2D position_texture;
//uniform sampler2D normal_texture;
//uniform sampler2D diffuse_texture;
layout(binding = 0) uniform sampler2D position_texture;
layout(binding = 1) uniform sampler2D normal_texture;
layout(binding = 2) uniform sampler2D diffuse_texture;

// Position of the camera in world space
uniform vec3 view_pos;

// Light data in the world
uniform AmbiantLight ambiant_light;

uniform uint point_light_nb;
uniform PointLight point_lights[MAX_LIGHTS];

uniform uint directional_lights_nb;
uniform DirectionalLight directional_lights[MAX_LIGHTS];


out vec4 FragColor;

vec3 sample_diffuse(vec2 uv) {
    return texture(diffuse_texture, uv).rgb;
}

float sample_specular(vec2 uv) {
    return max(1 - texture(diffuse_texture, uv).a, 1.0);
}

float calc_diffuse(vec3 light_direction, vec3 normal) {
    return max(dot(normal, light_direction), 0.0);
}

float calc_specular(vec3 light_direction, vec3 view_direction, vec3 normal) {
    vec3 halfway_direction = normalize(light_direction + view_direction);
    return pow(max(dot(normal, halfway_direction), 0.0), 32);
}

vec4 calc_ambiant_light(vec2 uv) {
    return vec4(ambiant_light.color * sample_diffuse(uv), 1.0) * ambiant_light.intensity;
}

/// Compute the color contribution of the given PointLight for the fragment
vec4 calc_point_light(PointLight pl, vec2 uv) {
    vec3 frag_pos = texture(position_texture, uv).rgb;
    vec3 normal = normalize(texture(normal_texture, uv).rgb);

    // Attenuation due to the distance from the point light
    // todo: Maybe use the sphere area equation
    float distance = length(pl.position - frag_pos);

    float attenuation = 0.0;
    if (pl.intensity > 0) {
        attenuation = pl.intensity / (pl.intensity + distance * distance);
    }

    vec3 light_direction = normalize(pl.position - frag_pos);
    vec3 view_direction = normalize(view_pos - frag_pos);

    float diffuse_amount = calc_diffuse(light_direction, normal);
    float specular_amount = calc_specular(light_direction, view_direction, normal);

    vec4 diffuse = vec4(pl.color * sample_diffuse(uv), 1.0);
    vec3 specular = pl.color * sample_specular(uv);

    //return vec4(distance / 10.0, distance / 10.0, distance / 10.0, 1.0);

    // todo : specular is probably not correct for transparent values
    return (diffuse * diffuse_amount + vec4(specular * specular_amount, sample_specular(uv))) * pl.intensity * attenuation;
}

/// Compute the color contribution of the given DirectionalLight for the fragment
vec4 calc_directional_light(DirectionalLight dl, vec2 uv) {
    vec3 frag_pos = texture(position_texture, uv).rgb;
    vec3 normal = normalize(texture(normal_texture, uv).rgb);

    vec3 light_direction = normalize(-dl.direction);
    vec3 view_direction = normalize(view_pos - frag_pos);

    float diffuse_amount = calc_diffuse(light_direction, normal);
    float specular_amount = calc_specular(light_direction, view_direction, normal);

    vec4 diffuse = vec4(dl.color * sample_diffuse(uv), 1.0);
    vec3 specular = dl.color * sample_specular(uv);

    return (diffuse * diffuse_amount + vec4(specular * specular_amount, sample_specular(uv))) * dl.intensity;
}

void main()
{
    vec2 uv = gl_FragCoord.xy / vec2(textureSize(diffuse_texture, 0));

    vec4 res = vec4(0.0);

    // Process ambient light
    res += calc_ambiant_light(uv);

    // Process point lights
    for (int i = 0; i < point_light_nb; i++) {
        res += calc_point_light(point_lights[i], uv);
    }

    // Process directional lights
    for (int i = 0; i < directional_lights_nb; i++) {
        res += calc_directional_light(directional_lights[i], uv);
    }

    FragColor = res;

    //FragColor = calc_point_light(point_lights[0], uv);
}

#endif