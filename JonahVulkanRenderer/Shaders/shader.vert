#version 450

struct Instance
{
	mat4 model;
	vec4 array_index;
};

layout(binding = 0) uniform UniformBufferObject{
    mat4 view;
    mat4 proj;
} ubo;

layout(std140, binding = 2) readonly buffer InstanceData {
    Instance instance_data[ ];
};

layout(std430, binding = 5) readonly buffer ShouldDraw {
    uint should_draw[ ];
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec3 outColor;
layout(location = 1) out vec3 outUV;

void main() {

    outColor = inColor;

    outUV = vec3(inUV, instance_data[gl_InstanceIndex].array_index.x);

    mat4 model = instance_data[gl_InstanceIndex].model;

    if(should_draw[gl_InstanceIndex] == 0){
        gl_Position = vec4(0,0,0,0);
    }else{
        gl_Position = ubo.proj * ubo.view * model* vec4(inPosition, 1.0);
    }
}