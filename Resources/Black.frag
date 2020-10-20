#version 330 core

in vec3 v_Position;
in vec3 v_Normal;

layout(location = 0) out vec4 fragColor;

void main()
{
    fragColor = vec4(0, 0, 0, 1);
}
