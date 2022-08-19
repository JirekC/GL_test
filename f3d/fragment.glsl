#version 330 core

in vec4 vertexColor; // input variable from vertex shader (same name and type)

out vec4 FragColor;

void main()
{
	if(vertexColor.a < 0.1)
		discard;
	FragColor = vertexColor;
}
