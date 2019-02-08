#version 410 core

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec2 inTexCoord;

uniform sampler2D albedoSampler;
uniform sampler2D bloomSampler;

layout(location = 0) out vec3 outColor;

uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

float blendOverlay(float base, float blend) {
	return base<0.5?(2.0*base*blend):(1.0-2.0*(1.0-base)*(1.0-blend));
}

vec3 blendOverlay(vec3 base, vec3 blend) {
	return vec3(blendOverlay(base.r,blend.r),blendOverlay(base.g,blend.g),blendOverlay(base.b,blend.b));
}

float blendSoftLight(float base, float blend) {
	return (blend<0.5)?(2.0*base*blend+base*base*(1.0-2.0*blend)):(sqrt(base)*(2.0*blend-1.0)+2.0*base*(1.0-blend));
}

vec3 blendSoftLight(vec3 base, vec3 blend) {
	return vec3(blendSoftLight(base.r,blend.r),blendSoftLight(base.g,blend.g),blendSoftLight(base.b,blend.b));
}

vec3 blendHardLight(vec3 base, vec3 blend) {
	return blendOverlay(blend,base);
}

vec3 blendHardLight(vec3 base, vec3 blend, float opacity) {
	return (blendHardLight(base, blend) * opacity + base * (1.0 - opacity));
}

void main()
{
    vec2 uv = inTexCoord;
    uv.y = 1 - uv.y;

    vec3 albedo = texture(albedoSampler, uv).rgb;
    vec3 bloom = texture(bloomSampler, uv).rgb;

    float exposure = 2.0;

//    albedo = pow(albedo, vec3(1/2.2));
//    bloom = pow(bloom, vec3(1/2.2));

    //bloom *= 0.1;
    //bloom = vec3(1) - exp(-bloom * .1);

//    albedo = 1 - exp(-albedo * (1.0 / exposure));

    // expand albedo into
    vec3 composite = albedo + bloom / 2;


//    composite *= exposure;

    vec3 final = composite;

    // Hejl-Burgess
//    vec3 x = max(vec3(0),final-0.004);
//    final = (x*(6.2*x+.5))/(x*(6.2*x+1.7)+0.06);

    // simple tone mapping to reduce HDR into LDR
    // [0,3] -> [0,1]
    final = vec3(1) - exp(-composite * exposure);

//    final = (clamp(composite * 2.2, 0, 1) + clamp(composite * .5, 0, 1) + clamp(composite * .2, 0, 1)) / 3.0;

    // simple gamma correct
//    final = pow(final, vec3(1/2.2));

    outColor = final;
}