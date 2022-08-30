#version 330 core

in vec3 fragment_pos; // input variables from vertex shader (same name and type)
in vec3 normal_vec;
uniform vec3 light_pos; // position of light source (same as position of camera)
uniform vec4 color; // object color

out vec4 FragColor;

const vec3 light_color = vec3(1.0f); // white light
const vec3 ambient_light = light_color * 0.2f; // color of ambient light

void main()
{
    vec3 light_direction = normalize(light_pos - fragment_pos);
    float diff = max(dot(normal_vec, light_direction), 0.0);
    vec3 diffuse_light = diff * light_color; // diffuse light component

    vec3 result = (ambient_light + diffuse_light) * vec3(color);
    FragColor = vec4(result, 1.0f);
}
