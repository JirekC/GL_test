#version 330 core

layout (location = 0) in vec3 model; // coordinates of cube instances
layout (location = 1) in vec3 normal; // normal vectors for single cube
layout (location = 2) in vec3 translate; // offset of one cube instance in instanced array
layout (location = 3) in float material; // material_nr

uniform mat4 view;
uniform mat4 transform;  // vertex: rotate, scale, translate of resulting object
uniform mat3 normal_mat; // norm-vactor transformations

out vec3 fragment_pos;  // forwarding vertex position in space - before view matrix application
out vec3 normal_vec;    // forwarding normalized normal-vector
out vec4 color;            // forwarding color based on material number

const vec4 palette[9] = vec4[]( vec4(0.1f, 0.1f, 0.1f, 1.0f), // mat #0
                                vec4(0.0f, 0.0f, 1.0f, 1.0f),
                                vec4(0.0f, 1.0f, 0.0f, 1.0f),
                                vec4(0.0f, 1.0f, 1.0f, 1.0f),
                                vec4(1.0f, 0.0f, 0.0f, 1.0f),
                                vec4(1.0f, 0.0f, 1.0f, 1.0f),
                                vec4(1.0f, 1.0f, 0.0f, 1.0f),
                                vec4(0.8f, 0.8f, 0.8f, 1.0f),
                                vec4(0.5f, 0.5f, 1.0f, 1.0f) );

void main()
{
    vec4 fp = transform * vec4(model + translate, 1.0f); // position in space
    fragment_pos = vec3(fp);
    gl_Position = view * fp; // view from camera
    normal_vec = normalize(normal_mat * normal);
    if(material <= 8.0f) {
        color = palette[int(material)];
    } else {
        color = vec4(material / 256.0f, 0.0f, 0.0f, 1.0f);
    }
}
