#version 450

struct Instance
{
	mat4 model;
	vec4 array_index;
};

layout(binding = 0) uniform UniformBufferObject{
    mat4 view;
    mat4 proj;
    Instance instance[2];
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec3 outColor;
layout(location = 1) out vec3 outUV;

void main() {
    outColor = inColor;
    outUV = vec3(inUV, ubo.instance[gl_InstanceIndex].array_index.x);
    mat4 model_view = ubo.view * ubo.instance[gl_InstanceIndex].model;
    gl_Position = ubo.proj * model_view * vec4(inPosition, 1.0);
}