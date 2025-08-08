#version 460 core
out vec4 FragColor;
in vec3 vColor;
void main()
{
    //FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
    FragColor = vec4(vColor, 1.0f);
}