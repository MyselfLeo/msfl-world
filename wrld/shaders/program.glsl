#version 460 core

#ifdef VERTEX_SHADER

layout (location = 0) in vec3 aPos;
//layout (location = 1) in vec3 aColor;
layout (location = 1) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 vColor;
out vec2 texCoord;

void main()
{
    gl_Position = projection * view * model* vec4(aPos, 1.0);
    //vColor = aColor;
    texCoord = aTexCoord;
}

#endif

#ifdef FRAGMENT_SHADER

out vec4 FragColor;
//in vec3 vColor;
in vec2 texCoord;

uniform sampler2D myTexture;

void main()
{
    //FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
    //FragColor = vec4(vColor, 1.0f);
    FragColor = texture(myTexture, texCoord);
}
#endif