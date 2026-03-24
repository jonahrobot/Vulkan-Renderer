#version 450

layout(location = 0) in vec4 in_position;
layout(location = 1) in vec3 in_color;
layout(location = 2) in vec3 in_normal;
layout(location = 3) in vec3 in_camera_position;

layout(push_constant) uniform LightData{
    vec4 light_color;
    vec4 light_position;
    vec4 light_mode;
} light_data;

layout(location = 0) out vec4 out_color;

void main() {

    // Phong shading

    // Ambient
    vec3 ambient = vec3(0.859,0.506,0.2);
    
    // Diffuse
    vec3 normal = normalize(in_normal);
    vec3 light_color = light_data.light_color.xyz;
    vec3 light_direction = normalize(light_data.light_position.xyz - in_position.xyz);
    float diffuse_strength = max(0.0, dot(normal, light_direction));
    vec3 diffuse = diffuse_strength * vec3(0.596,0.325,0.722);

    // Specular
    vec3 specular = vec3(0.0,0.0,0.0);
    if(diffuse_strength > 0.0){
        vec3 view_position = normalize(in_camera_position - in_position.xyz);
        vec3 reflection_position = reflect(-light_direction, normal);
        float specular_strength = max(0.0, dot(reflection_position, view_position));
        specular_strength = pow(specular_strength, 32.0);
        specular = specular_strength * vec3(1,1,1);
    }

    // Lighting sum 
    vec3 lighting = ambient * 0 + diffuse * 1 + specular * 1;

    // Rendering
    vec3 model_color = in_color;
    out_color = vec4(model_color * lighting, 1.0);
}