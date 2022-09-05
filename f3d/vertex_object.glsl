#version 330 core

layout (location = 0) in vec3 model; // vertex
layout (location = 1) in vec3 normal; // normal vector
uniform mat4 view;
uniform mat4 transform;  // vertex:      rotate, scale, translate
uniform mat3 normal_mat; // norm-vactor: rotate, scale, translate

out vec3 fragment_pos;  // forwarding vertex position in space - before view matrix application
out vec3 normal_vec;    // forwarding normalized normal-vector

void main()
{
    vec4 fp = transform * vec4(model, 1.0f); // position in space
    fragment_pos = vec3(fp);
    gl_Position = view * fp; // view from camera
    normal_vec = normalize(normal_mat * normal);
}
