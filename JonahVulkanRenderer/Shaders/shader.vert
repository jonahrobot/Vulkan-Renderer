#version 450

layout(binding = 0) uniform UniformBufferObject{
    mat4 view;
    mat4 proj;
} ubo;

struct Instance
{
	mat4 model;
	vec4 array_index;
};

layout(std140, binding = 1) readonly buffer InstanceData {
    Instance instance_data[ ];
};

layout(std430, binding = 3) readonly buffer ShouldDraw {
    uint should_draw[ ];
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 outColor;

void main() {

    outColor = inColor;

    mat4 model = instance_data[gl_InstanceIndex].model;

    if(should_draw[gl_InstanceIndex] == 0){
        gl_Position = vec4(0,0,0,0);
    }else{
        gl_Position = ubo.proj * ubo.view * model* vec4(inPosition, 1.0);
    }
}