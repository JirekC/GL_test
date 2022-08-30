#version 330 core

layout (location = 0) in vec3 model; // vertex
layout (location = 1) in vec3 normal; // normal vector
layout (location = 2) in vec3 translate; // offset of model (cube) instance in instanced array

uniform mat4 view;
uniform mat4 transform;  // vertex: rotate, scale, translate of resulting object
uniform mat3 normal_mat; // norm-vactor transformations

out vec3 fragment_pos;  // forwarding vertex position in space - before view matrix application
out vec3 normal_vec;	// forwarding normalized normal-vector

void main()
{
	vec4 fp = transform * vec4(model + translate, 1.0f); // position in space
	fragment_pos = vec3(fp);
	gl_Position = view * fp; // view from camera
	normal_vec = normalize(normal_mat * normal);
}
