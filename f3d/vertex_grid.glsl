#version 330 core

layout (location = 0) in vec4 model; // position of vertices in model

uniform mat4 view; // view matrix (multiplied rotate, translate and scale)
uniform vec4 color;

out vec4 vertexColor;

void main()
{
	gl_Position = view * model; // rot, trans, scale ...
	vertexColor = color; // propagate color to fragment shader
}
