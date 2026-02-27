#version 450

// -- Data --

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

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_color;
layout(location = 2) in vec3 in_normal;

layout(location = 0) out vec4 out_position;
layout(location = 1) out vec3 out_color;
layout(location = 2) out flat vec3 out_normal;
layout(location = 3) out vec3 out_camera_pos;

// -- Main --

void main() {

    // Culling 

    mat4 instance_model_matrix = instance_data[gl_InstanceIndex].model;

    if(should_draw[gl_InstanceIndex] == 0){
        gl_Position = vec4(0,0,0,0);
    }else{
        vec4 position = ubo.proj * ubo.view * instance_model_matrix * vec4(in_position, 1.0);
        gl_Position = position;
    }

    // Setup fragment shader
    out_position = ubo.view * instance_model_matrix * vec4(in_position, 1.0);
    out_color = in_color;
    out_normal = mat3(instance_model_matrix) * in_normal;
    out_camera_pos = inverse(ubo.view)[3].xyz;
}