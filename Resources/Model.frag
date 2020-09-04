#version 330 core

in vec3 v_Position;
in vec3 v_Normal;

layout(location = 0) out vec4 fragColor;

void main()
{
    vec3 lightPos = vec3(0, 3, 0);
    vec3 L = normalize(lightPos - v_Position);
    vec3 N = v_Normal;
    float NdotL = max(dot(N, L), 0);
    fragColor = vec4(vec3(NdotL), 1);
}
