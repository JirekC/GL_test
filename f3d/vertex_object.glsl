#version 330 core

layout (location = 0) in vec3 model; // position of vertices in model (cube)
layout (location = 1) in vec3 normal; // offset of model (cube) instance in instanced array

uniform mat4 view;
uniform vec4 color;
uniform vec3 light_pos;

out vec4 vertexColor; // color output to the fragment shader

void main()
{
	gl_Position = view * vec4( model.x, model.y, model.z, 1.0f );
	//gl_PointSize = 10 * (5 - gl_Position.z); // size of point if GL_POINTS draw style selected
	vec3 norm = normalize(normal);
	vec3 lightDir = normalize(light_pos - vec3(gl_Position)); 
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * vec3(1.0f); // while light
	vertexColor = vec4(vec3(diffuse * vec3(color)), color.w);
}
