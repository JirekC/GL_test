#include <iostream>
#include <fstream>
#include <cstdint>
#include "object_creator.hpp"
#include "f3d_math.hpp"

using namespace f3d;

/*
STL file format (binary):
UINT8[80]    – Header                 - 80 bytes
UINT32       – Number of triangles    -  4 bytes
foreach triangle                      - 50 bytes:
    REAL32[3] – Normal vector             - 12 bytes
    REAL32[3] – Vertex 1                  - 12 bytes
    REAL32[3] – Vertex 2                  - 12 bytes
    REAL32[3] – Vertex 3                  - 12 bytes
    UINT16    – Attribute byte count      -  2 bytes
end
 */
object_3d_vi* loader::LoadSTL(const char* path)
{
    char rdata[80]; // biggest chunk of data is header (80 Bytes)
    uint32_t nr_of_triangles;
    unsigned int indices_cntr = 0;
    std::ifstream data_file;
    object_3d_vi* model = new object_3d_vi;

    try
    {
        data_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        data_file.open(path, std::ios::binary);
        data_file.read(rdata, 80); // read header and flush it
        data_file.read((char*)&nr_of_triangles, 4); // assume little-endian machine (x86 is)
        model->vertices.reserve(nr_of_triangles * 3 * 3); // 3 vertices * 3 coordinates per triangle
        model->normals.reserve(nr_of_triangles * 3 * 3);
        model->indices.reserve(nr_of_triangles * 3); // memory waste, but ..
        // for each triangle
        for(uint32_t i = 0; i < nr_of_triangles; i++)
        {
            float norm_vec[3]; // assume little-endian machine (x86 is)
            float vertex[9]; // x1, y1, z1, x2, y2 .. z3
            data_file.read((char*)norm_vec, sizeof(float) * 3); // normal vector, same for all 3 vertices
            data_file.read((char*)vertex, sizeof(float) * 9); // all 3 vertexes
            data_file.read(rdata, 2); // flush "Attribute byte count"
            model->vertices.insert(model->vertices.end(), vertex, vertex + 9);
            model->normals.insert(model->normals.end(), norm_vec, norm_vec + 3); // same normal for each vertice in STL
            model->normals.insert(model->normals.end(), norm_vec, norm_vec + 3);
            model->normals.insert(model->normals.end(), norm_vec, norm_vec + 3);
            model->indices.push_back(indices_cntr++); // SLT does not use indices (each triangle has separated 3 vertices)
            model->indices.push_back(indices_cntr++); // so simply storing 0, 1, 2 ... for indices when loading from STL
            model->indices.push_back(indices_cntr++);
        }
        data_file.close();
    }
    catch(const std::exception& e)
    {
        std::string s = "ERROR::f3d::loader::LoadSTL(): FILE_NOT_SUCCESFULLY_READ: " + std::string(path) + ":\n";
        throw std::runtime_error(s);
    }

    return model;    
}

void object3d::Prepare( object_3d_vi* model,
                        glm::u32vec3 position,
                        glm::vec3 rotation,
                        glm::u32vec3 size,
                        glm::vec4 color )
{
    this->size = size;
    this->rotation = rotation;
    this->position = position;
    this->color = color;

    glGenVertexArrays(1, &vao);
	glBindVertexArray(vao); // bind Vertex Array Object
	glGenBuffers(3, buff);
	// vertices
	glBindBuffer(GL_ARRAY_BUFFER, buff[0]); // copy vertices array to buffer for OpenGL use
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * model->vertices.size(), model->vertices.data(), GL_STATIC_DRAW);
    // then set our vertex attributes pointers (tell OpenGL how to interpred vertex data)
    glVertexAttribPointer(
        0, // attribute "location = 0" in shader
        3, // size of attribute (in floats, vertex coord is vec3)
        GL_FLOAT,
        GL_FALSE,
        3 * sizeof(float), // space between consecutive vertex attributes
        (void*)0 // offset of this attribute in vertices "structure"
    );
    glEnableVertexAttribArray(0);
    // normals
	glBindBuffer(GL_ARRAY_BUFFER, buff[1]); // copy vertices array to buffer for OpenGL use
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * model->normals.size(), model->normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(
        1, // attribute "location = 1" in shader
        3, // size of attribute (in floats, normal vector is vec3)
        GL_FLOAT,
        GL_FALSE,
        3 * sizeof(float), // space between consecutive normal attributes
        (void*)(3 * sizeof(float)) // offset of this attribute in vertices "structure"
    );
    glEnableVertexAttribArray(1);
    // indices
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buff[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * model->indices.size(), model->indices.data(), GL_STATIC_DRAW);
    nr_of_vertices = model->indices.size(); // yes, total number of vertices in final model is equal to nr of indices
    delete model; // no more needed, everythinh is in GPU
}

void object3d::Draw(const glm::mat4& view_matrix)
{
    _shader->use();
    // Create transformationmatrix from rotation, size and position(translation)
    // than multiply with view_matrix before pass to GPU
    glm::mat4 trans = glm::mat4(1.0f);
    trans = glm::translate(trans, position);
    trans = f3d::RotationMatrix(rotation) * trans;
    trans = glm::scale(trans, size);
    _shader->setUniform("view", view_matrix * trans);
    _shader->setUniform("color", color);
    _shader->setUniform("light_pos", glm::vec3({256,256,256})); // TODO: use real value

    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, nr_of_vertices, GL_UNSIGNED_INT, 0);
}
