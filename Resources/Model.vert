#version 330 core

uniform mat4 projMatrix;

layout(location = 0) in vec4 position;

out vec3 v_Position;

void main()
{
    gl_Position = projMatrix * position;
    //v_Position = (modelMatrix * position).xyz;
}
