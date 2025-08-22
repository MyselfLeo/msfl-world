#version 460 core

// From viewport to world (viewport -> projection -> view)
uniform mat4 inv_matrix;
uniform vec3 camera_pos;
uniform samplerCube cubemap;

out vec4 FragColor;

void main()
{
    vec4 p = inv_matrix * vec4(gl_FragCoord.xyz, 1);
    vec3 pixel_pos = p.xyz / p.w;

    vec3 direction = normalize(pixel_pos - camera_pos);

    FragColor = texture(cubemap, direction);
}