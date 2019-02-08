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

    // this log is inverse of the tone-mapping exponential function used during composition
    // it preseves the gamma of the original albedo color
    // [0,1] -> [0,3]
    outColor = -log(1 - color) / 2.0;
//    outColor = vec3(inTexCoord, 0);

    const float maxBrightness = 1.75; // a bit bigger than max length of vec3 (sqrt(x^2 + y^2 + z^2))
    const float threshold = 1.5; // min brightness to be included in bloom
    const float expansionRatio = 1.0 / (maxBrightness - threshold); // multiplier to expand to [0-1] range

    // highlights are written out in [0,1] range
    float brightness = length(color);
    outHighlights = clamp(outColor * (brightness - threshold) * expansionRatio, 0, 1);
}