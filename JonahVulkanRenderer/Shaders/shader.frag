#version 450

layout(location = 0) in vec3 in_color;
layout(location = 1) in vec3 in_position;
layout(location = 2) in vec3 in_normal;

layout(location = 0) out vec4 out_color;

// Sourced: https://www.cs.toronto.edu/~jacobson/phong-demo/

void main() {

    float ka = 1.0;
    float kd = 1.0;
    float ks = 1.0;
    float shine = 80;

    vec3 ambient_color = vec3(52,25,0);
    vec3 diffuse_color = vec3(204,102,0);
    vec3 specular_color = vec3(255,255,255);
    vec3 light_pos = vec3(2.2,1,-1);

    vec3 N = in_normal;
    vec3 L = normalize(light_pos - in_position);

    // Lambert Cosine Law
    float lambertian = max(dot(N,L), 0.0);
    float specular = 0.0;
    if(lambertian > 0.0){
        vec3 R = reflect(-L,N);
        vec3 V = normalize(-in_position);
        float spec_angle = max(dot(R,V),0.0);
        specular = pow(spec_angle, shine);
    }

    out_color = vec4(ka * ambient_color +
                      kd * lambertian * diffuse_color +
                      ks * specular * specular_color, 1.0);

    // out_color = vec4(in_color,1.0);
}