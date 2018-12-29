#version 410 core

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec2 inTexCoord;

uniform sampler2D texSampler;

out vec4 outColor;

void main()
{
    vec3 tex = texture(texSampler, inTexCoord).rgb;
    outColor = vec4(tex, 1.0);
}