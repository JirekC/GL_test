#version 330 core

in vec2 tex_coord; // input variable from vertex shader (same name and type)

uniform sampler2D texture_of_values;

out vec4 FragColor;

const float color_gain = 1.0f;

void main()
{
    float s_value = texture(texture_of_values, tex_coord).r;
    s_value = (s_value * 2.0f) - 1.0f; // textures are saturated by OpenGL to 0 .. 1
    s_value = clamp(color_gain * s_value, -1.0f, 1.0f);
    FragColor = vec4((s_value < 0 ? 0.12f * (-s_value) : s_value), 1.0f + s_value , 1.0f - (s_value < 0 ? 0.5f * s_value : s_value), 1.0f);
}
