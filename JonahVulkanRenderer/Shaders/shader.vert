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

layout(location = 0) out vec3 out_color;
layout(location = 1) out vec3 out_position;
layout(location = 2) out vec3 out_normal;

// -- Main --

void main() {

    mat4 model = instance_data[gl_InstanceIndex].model;

    vec4 vert_pos_model = ubo.view * model * vec4(in_position, 1.0);

    if(should_draw[gl_InstanceIndex] == 0){
        gl_Position = vec4(0,0,0,0);
    }else{
        gl_Position = ubo.proj * vert_pos_model;
    }

    out_position = vec3(vert_pos_model) / vert_pos_model.w;
    out_normal = in_normal;
}