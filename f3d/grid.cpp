#include <iostream>
#include "grid.hpp"

#define LOG_10 (2.302585093) /* ln_e(10) */

/**
 * @brief Calculates decadic order (exponent in exponential form) of individual numbers in vector.
 * 
 * @param numbers Vector with input numbers
 * @return glm::vec<3, uint32_t> order of each number (input-vector element)
 */
inline glm::vec<3, uint32_t> NumberOrder(glm::vec3 numbers)
{
    glm::vec<3, uint32_t> order;
    // trim pecimal places by float -> int conversion
    order = glm::log(numbers) / (float)LOG_10;
    return order;
}

/**
 * @brief 
 * 
 * @param scene_size    Total scene size in individual directions
 * @param nr_of_lines   Wanted number of lines per whole scene in individual directions
 */
void f3d::grid::Prepare(glm::vec3 scene_size, glm::vec<3, uint32_t> nr_of_lines)
{
    std::vector<float> vertices; // temporary before copying to GPU's memory
    line_spacing = scene_size / glm::vec3(nr_of_lines); // assumed spaces between individual lines
    glm::vec3 line_spacing_exponent = NumberOrder(line_spacing); // decadic order of line_spacing
    glm::vec3 line_spacing_mantisa = line_spacing / glm::pow(glm::vec3(10.0f), line_spacing_exponent);
    // "glue" mantisa to one of common numbers 1, 2 or 5
    for(int i = 0; i < 3; i++)
    {
        if(line_spacing_mantisa[i] < 2.0f)
            line_spacing_mantisa[i] = 1.0f;
        else if (line_spacing_mantisa[i] < 5.0f)
            line_spacing_mantisa[i] = 2.0f;
        else
            line_spacing_mantisa[i] = 5.0f;
    }
    // store new line_spacing and update number of lines
    line_spacing = line_spacing_mantisa * glm::pow(glm::vec3(10.0f), line_spacing_exponent);
    nr_of_lines = scene_size / line_spacing;

    // generate vertices - using GL_LINES draw-mode, so lines are created from vertices l1:(v0,v1), l2:(v2,v3) ...
    for(uint32_t xi = 0; xi < nr_of_lines.x; xi++)
    {
        // parallel to axis Y
        vertices.push_back(line_spacing.x * xi);    // x0
        vertices.push_back(0.0f);                   // y0
        vertices.push_back(0.0f);                   // z0
        vertices.push_back(1.0f);                   // w0
        vertices.push_back(line_spacing.x * xi);    // x1
        vertices.push_back(scene_size.y);           // y1
        vertices.push_back(0.0f);                   // z1
        vertices.push_back(1.0f);                   // w1
        // parallel to axis Z
        vertices.push_back(line_spacing.x * xi);    // x0
        vertices.push_back(0.0f);                   // y0
        vertices.push_back(0.0f);                   // z0
        vertices.push_back(1.0f);                   // w0
        vertices.push_back(line_spacing.x * xi);    // x0
        vertices.push_back(0.0f);                   // y0
        vertices.push_back(scene_size.z);           // z0
        vertices.push_back(1.0f);                   // w0
    }
    for(uint32_t yi = 0; yi < nr_of_lines.y; yi++)
    {
        // parallel to axis X
        vertices.push_back(0.0f);                   // x0
        vertices.push_back(line_spacing.y * yi);    // y0
        vertices.push_back(0.0f);                   // z0
        vertices.push_back(1.0f);                   // w0
        vertices.push_back(scene_size.x);           // x1
        vertices.push_back(line_spacing.y * yi);    // y1
        vertices.push_back(0.0f);                   // z1
        vertices.push_back(1.0f);                   // w1
        // parallel to axis Z
        vertices.push_back(0.0f);                   // x0
        vertices.push_back(line_spacing.y * yi);    // y0
        vertices.push_back(0.0f);                   // z0
        vertices.push_back(1.0f);                   // w0
        vertices.push_back(0.0f);                   // x0
        vertices.push_back(line_spacing.y * yi);    // y0
        vertices.push_back(scene_size.z);           // z0
        vertices.push_back(1.0f);                   // w0
    }
    for(uint32_t zi = 0; zi < nr_of_lines.z; zi++)
    {
        // parallel to axis X
        vertices.push_back(0.0f);                   // x0
        vertices.push_back(0.0f);                   // y0
        vertices.push_back(line_spacing.z * zi);    // z0
        vertices.push_back(1.0f);                   // w0
        vertices.push_back(scene_size.x);           // x1
        vertices.push_back(0.0f);                   // y1
        vertices.push_back(line_spacing.z * zi);    // z1
        vertices.push_back(1.0f);                   // w1
        // parallel to axis Y
        vertices.push_back(0.0f);                   // x0
        vertices.push_back(0.0f);                   // y0
        vertices.push_back(line_spacing.z * zi);    // z0
        vertices.push_back(1.0f);                   // w0
        vertices.push_back(0.0f);                   // x0
        vertices.push_back(scene_size.y);           // y0
        vertices.push_back(line_spacing.z * zi);    // z0
        vertices.push_back(1.0f);                   // w0
    }
    nr_of_vertices = vertices.size();

    /*** OpenGL part ***/
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao); // bind Vertex Array Object
	glGenBuffers(1, &buff);
	// vertices
	glBindBuffer(GL_ARRAY_BUFFER, buff); // copy vertices array to buffer for OpenGL use
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
    // vertices - 3D coordinates
    glVertexAttribPointer(
        0, // attribute "location = 0" in shader
        4, // size of attribute (in floats, vertex coord is vec4)
        GL_FLOAT,
        GL_FALSE,
        4 * sizeof(float), // space between consecutive vertex attributes
        (void*)0 // offset of this attribute in vertices "structure"
    );
    glEnableVertexAttribArray(0);
}

void f3d::grid::Draw(const glm::mat4& view_matrix)
{
    _shader->use();
    _shader->setUniform("view", view_matrix);
    _shader->setUniform("color", color);

    glBindVertexArray(vao);
    glDrawArrays(GL_LINES, 0, nr_of_vertices);
}
