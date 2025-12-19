#version 450

layout(binding = 1) uniform sampler2DArray texSamplerArray;

layout(location = 0) in vec3 inColor;
layout(location = 1) in vec3 inUV;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(inColor,1.0);
    //texture(texSamplerArray, inUV);
}