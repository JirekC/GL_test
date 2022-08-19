#version 330 core

layout (location = 0) in vec4 model; // position of vertices in model (rectangle)
layout (location = 1) in vec2 tex_coord_in; // texture coordinates

uniform mat4 view; // view matrix (multiplied rotate, translate and scale)

out vec2 tex_coord;

float s_value;

void main()
{
	gl_Position = view * model; // rot, trans, scale ...
	tex_coord = tex_coord_in; // propagate texture coordinates to fragment shader
}
