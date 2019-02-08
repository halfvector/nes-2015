#version 410 core

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec2 inTexCoord;

uniform sampler2D texSampler;

layout(location = 0) out vec3 outColor;

void main()
{
    vec3 tex = texture(texSampler, inTexCoord).rgb;
    outColor = tex;
}