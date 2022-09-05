#version 330 core

in vec2 tex_coord; // input variable from vertex shader (same name and type)

uniform sampler2D texture_of_values;

out vec4 FragColor;

void main()
{
    float value = texture(texture_of_values, tex_coord).r; // TODO: textures are already saturated by OpenGL to 0 .. 1
    // saturate value to <1, -1>
    float s_value = clamp(value, -1.0f, 1.0f);
    FragColor = vec4((s_value < 0 ? 0.12f * (-s_value) : s_value), 1.0f + s_value , 1.0f - (s_value < 0 ? 0.5f * s_value : s_value), 1.0f);
    //FragColor = vec4(1, 1, 1, value);
}
