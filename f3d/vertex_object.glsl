#version 330 core

layout (location = 0) in vec4 model; // position of vertices in model (cube)
layout (location = 1) in vec4 translate; // offset of model (cube) instance in instanced array

uniform mat4 view;
uniform vec4 color;

out vec4 vertexColor; // color output to the fragment shader

void main()
{
	gl_Position = view * ( translate + model );
	//gl_PointSize = 10 * (5 - gl_Position.z); // size of point if GL_POINTS draw style selected
	vertexColor = color;
}
