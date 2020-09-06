#version 330 core

in vec3 v_Position;
in vec3 v_Normal;
in vec2 v_TexCoord;

layout(location = 0) out vec4 fragColor;

float checker(vec2 uv, float repeats) 
{
  float cx = floor(repeats * uv.x);
  float cy = floor(repeats * uv.y); 
  float result = mod(cx + cy, 2.0);
  return sign(result);
}

void main()
{
    vec2 uv = v_TexCoord;
    float fog = min(1, length(uv * 2 - 1));
    uv *= 40;
    float a = sign(mod(floor(uv.x) + floor(uv.y), 2.0));
    float c = mix(1.0, 0.9, a);
    float f = mix(c, 0.9, fog);
    fragColor = vec4(vec3(f), 1);
};
