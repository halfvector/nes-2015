#version 410 core

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec2 inTexCoord;

uniform sampler2D texSampler;

layout(location = 0) out vec3 outColor;
layout(location = 1) out vec3 outHighlights;

void main()
{
    vec2 texCoord = inTexCoord;
    texCoord.y = 1 - texCoord.y;

    vec3 color = texture(texSampler, texCoord).rgb;
    outColor = color;

    float brightness = length(color);
    outHighlights = brightness > 1.4 ? color : vec3(0);
}