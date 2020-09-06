#version 330 core

uniform mat4 projMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

out vec3 v_Position;
out vec3 v_Normal;
out vec2 v_TexCoord;

void main()
{
    gl_Position = projMatrix * viewMatrix * modelMatrix * position;
    v_Position = (modelMatrix * position).xyz;
    v_Normal = (modelMatrix * vec4(normal, 0)).xyz;
    v_TexCoord = texCoord;
}
