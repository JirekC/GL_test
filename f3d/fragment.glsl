#version 330 core

out vec4 FragColor;
in vec4 vertexColor; // input variable from vertex shader (same name and type)

void main()
{
	if(vertexColor.a < 0.1)
		discard;
	FragColor = vertexColor;
}
