#version 410 core

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec2 inTexCoord;

uniform sampler2D texSampler;
uniform bool horizontalBlur = false;

layout(location = 0) out vec3 outColor;

// 1D gaussian with sigma=1 and size=11
const float weight[] = float[] (0.198596,0.175713,0.121703,0.065984,0.028002,0.0093);

void main()
{
    vec2 tex_offset = 0.2 / textureSize(texSampler, 0);
    vec2 uv = inTexCoord;
    uv.y = 1 - uv.y;

    vec3 result = texture(texSampler, uv).rgb * weight[0];

    // only blur in one dimension based on shader settings
    // this gives us O(2N) instead of O(N^2) texture reads per pixel
    // thanks to the separable nature of a 2D gaussian filter
    if(horizontalBlur) {
        for(int i = 1; i <= 6; i++) {
            result += texture(texSampler, uv + vec2(tex_offset.x * i, 0)).rgb * weight[i];
            result += texture(texSampler, uv - vec2(tex_offset.x * i, 0)).rgb * weight[i];
        }
    } else {
        for(int i = 1; i <= 6; i++) {
            result += texture(texSampler, uv + vec2(0, tex_offset.y * i)).rgb * weight[i];
            result += texture(texSampler, uv - vec2(0, tex_offset.y * i)).rgb * weight[i];
        }
    }

    outColor = result;
}