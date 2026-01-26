//
// Created by leo on 1/26/26.
//

#pragma once

#include <string>

namespace wrld::shader {
    inline std::string TAA_SECOND_PASS = R"(
#version 460 core

#ifdef VERTEX_SHADER

void main()
{
    // Triangle covering the whole screen
    vec2 vertices[3] = vec2[3](vec2(-1, -1), vec2(3, -1), vec2(-1, 3));
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

layout (binding = 0) uniform sampler2D position_texture;
layout (binding = 1) uniform sampler2D normal_texture;
layout (binding = 2) uniform sampler2D diffuse_texture;
layout (binding = 3) uniform usampler2D do_lighting_texture;

// TAA uniforms, history texture & alpha value
layout (binding = 4) uniform sampler2D history_texture;
uniform float min_alpha;
uniform bool clear_alpha;
uniform mat4 previous_viewproj;

// Position of the camera in world space
uniform vec3 view_pos;

// Light data in the world
uniform AmbiantLight ambiant_light;

uniform uint point_light_nb;
uniform PointLight point_lights[MAX_LIGHTS];

uniform uint directional_lights_nb;
uniform DirectionalLight directional_lights[MAX_LIGHTS];

uniform float elapsed_time;
uniform vec2 window_size;
uniform uint taa_frame_count;

out vec4 FragColor;

// Hash x (x = seed, returns a random number)
// x must be odd.
uint hash(uint x)
{
    x ^= x >> 16;
    x *= 0x21f0aaad;
    x ^= x >> 15;
    x *= 0xd35a2d97;
    x ^= x >> 15;
    return x;
}

// Returns a random number between 0 and 1.
float random(uint seed) {
    seed = (seed << 1) | uint(1);
    return float(hash(seed)) / 4294967296.0;
}

uint elapsed_ms() {
    return uint(elapsed_time * 1000);
}

uint do_lighting(vec2 uv) {
    return texture(do_lighting_texture, uv).r;
}

vec3 sample_diffuse(vec2 uv) {
    return texture(diffuse_texture, uv).rgb;
}

vec4 sample_history(vec2 uv) {
    return texture(history_texture, uv);
}

float sample_specular(vec2 uv) {
    return min(texture(diffuse_texture, uv).a, 1.0);
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

/// Return a boolean that tells if the reprojection was successful
/// and the history value can be used for accumulation.
/// If true, the value is put in history.
bool validate_history(vec3 curr_color, vec2 uv, out vec4 history) {
    if (clear_alpha) {
        return false;
    }

    // Reproject frag position
    vec4 world_pos = texture(position_texture, uv);
    vec4 prev_clip = previous_viewproj * world_pos;

    if (abs(prev_clip.w) < 1e-6) {
        return false; // invalid
    }

    vec2 prev_ndc = prev_clip.xy / prev_clip.w;
    vec2 old_uv = prev_ndc * 0.5 + 0.5;

    if (any(lessThan(old_uv, vec2(0.0))) || any(greaterThan(old_uv, vec2(1.0)))) {
        return false; // outside screen
    }

    // Invalid diffuse
    if (sample_diffuse(uv) != sample_diffuse(old_uv)) {
        return false;
    }

    history = sample_history(old_uv);

    return true;
}

void main()
{
    uint idx = uint(gl_FragCoord.y) * uint(window_size.x) + uint(gl_FragCoord.x);
    uint depth = uint(gl_FragCoord.z * 4294967296.0);
    uint seed = elapsed_ms() * idx * depth;

    vec2 uv = gl_FragCoord.xy / vec2(textureSize(diffuse_texture, 0));

    if (do_lighting(uv) == 0) {
        FragColor = vec4(sample_diffuse(uv), 1.0);
        return;
    }

    vec4 res = vec4(0.0);

    // Process ambient light
    res += calc_ambiant_light(uv);

    // Process point lights
    for (int i = 0; i < point_light_nb; i++) {
        float rdm = random(++seed);
        if (rdm > (1.0 / taa_frame_count)) continue;

        res += calc_point_light(point_lights[i], uv);
    }

    // Process directional lights
    for (int i = 0; i < directional_lights_nb; i++) {
        float rdm = random(++seed);
        if (rdm > (1.0 / taa_frame_count)) continue;

        res += calc_directional_light(directional_lights[i], uv);
    }

    // TAA accumulation
    vec4 history_color = vec4(0);
    bool use_history = validate_history(res.rgb, uv, history_color);

    if (use_history) {
        uint n = uint(history_color.w * 255);
        float dyn_alpha = 1.0 / float(n + 1);
        float alpha = max(min_alpha, dyn_alpha);
        vec3 color = (alpha * res.rgb) + ((1 - alpha) * history_color.rgb);
        FragColor = vec4(color, history_color.w + (1.0 / 255.0));
    } else {
        FragColor = vec4(res.rgb, 0);
    }
}

#endif
)";
}
