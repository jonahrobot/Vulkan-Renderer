#version 450

layout(location = 0) in vec4 in_position;
layout(location = 1) in vec3 in_color;
layout(location = 2) in vec3 in_normal;
layout(location = 3) in vec3 in_camera_position;

layout(location = 0) out vec4 out_color;

void main() {

    // Phong shading

    // Ambient
    vec3 ambient = vec3(0.5,0.5,0.5);
    
    // Diffuse
    vec3 normal = normalize(in_normal);
    vec3 light_color = vec3(1.0, 1.0, 1.0);
    vec3 light_position = vec3(1.0, 1.0, 1.0);
    float diffuse_strength = max(0.0, dot(light_position, normal));
    vec3 diffuse = diffuse_strength * light_color;

    // Specular
    vec3 camera_position = in_camera_position;
    vec3 view_position = normalize(camera_position);
    vec3 reflection_position = normalize(reflect(-light_position, normal));
    float specular_strength = max(0.0, dot(view_position, reflection_position));
    specular_strength = pow(specular_strength, 32.0);
    vec3 specular = specular_strength * light_color;

    // Lighting sum 
    vec3 lighting = ambient * 0.01 + diffuse * 0.5 + specular * 0.5;

    // Rendering
    vec3 model_color = in_color;
    out_color = vec4(model_color * lighting, 1.0);
}