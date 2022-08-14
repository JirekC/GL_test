#version 330 core

layout (location = 0) in vec4 model; // position of vertices in model (cube)
layout (location = 1) in vec4 translate; // offset of model (cube) instance in instanced array
layout (location = 2) in float value;

uniform mat4 view;

out vec4 vertexColor; // color output to the fragment shader

float s_value;

void main()
{
	gl_Position = view * ( translate + model );
	//gl_PointSize = 10 * (5 - gl_Position.z); // size of point if GL_POINTS draw style selected
	//vertexColor = value > 0.1f ? vec4(0.5f , 0.7f , 0.5f , 1.0f) : vec4(0.0f);
	//vertexColor = vec4(0.0f, value, 0.0f, 1.0f);
	//vertexColor = vec4(0.0f, value , -value, 1.0f);
	// TODO: saturate value to <1, -1>
	s_value = clamp(value, -1.0f, 1.0f);
	//vertexColor = value < 0.0f ? vec4(0.25f * (1.0f + value), 1.0f + value , 0.5f * (1.0f + value), 1.0f) : vec4(value, 1.0f , 1.0f - value, 1.0f);
	vertexColor = vec4((s_value < 0 ? 0.12f * (-s_value) : s_value), 1.0f + s_value , 1.0f - (s_value < 0 ? 0.5f * s_value : s_value), 1.0f);
}
