#version 410 core
//#extension GL_ARB_separate_shader_objects : enable

uniform mat4 MVP; // model view projection matrix

layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 texCoord;

// build-in vertex shader outputs
out vec4 gl_Position;

// our outputs
layout(location = 0) out vec3 outColor;
layout(location = 1) out vec2 outTexCoord;

void main()
{
    outColor = color;
    outTexCoord = texCoord;
    gl_Position = MVP * vec4(position, 0.0, 1.0);
}