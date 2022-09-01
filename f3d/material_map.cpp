#include <fstream>
#include <iostream>
#include <vector>
#include <stdint.h>
#include "material_map.hpp"

using namespace std;

float vertices[] = {
	// Back face
	-0.5f, -0.5f, -0.5f,
	0.5f,  0.5f, -0.5f,
	0.5f, -0.5f, -0.5f,
	0.5f,  0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,
	-0.5f,  0.5f, -0.5f,
	// Front face
	-0.5f, -0.5f,  0.5f,
	0.5f, -0.5f,  0.5f,
	0.5f,  0.5f,  0.5f,
	0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f,  0.5f,
	-0.5f, -0.5f,  0.5f,
	// Left face
	-0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f, -0.5f,
	-0.5f, -0.5f,  0.5f,
	-0.5f,  0.5f,  0.5f,
	// Right face
	0.5f,  0.5f,  0.5f,
	0.5f, -0.5f, -0.5f,
	0.5f,  0.5f, -0.5f,
	0.5f, -0.5f, -0.5f,
	0.5f,  0.5f,  0.5f,
	0.5f, -0.5f,  0.5f,
	// Bottom face
	-0.5f, -0.5f, -0.5f,
	0.5f, -0.5f, -0.5f,
	0.5f, -0.5f,  0.5f,
	0.5f, -0.5f,  0.5f,
	-0.5f, -0.5f,  0.5f,
	-0.5f, -0.5f, -0.5f,
	// Top face
	-0.5f,  0.5f, -0.5f,
	0.5f,  0.5f,  0.5f,
	0.5f,  0.5f, -0.5f,
	0.5f,  0.5f,  0.5f,
	-0.5f,  0.5f, -0.5f,
	-0.5f,  0.5f,  0.5f
};

float normals[] = {
	0.0f,  0.0f, -1.0f,
	0.0f,  0.0f, -1.0f,
	0.0f,  0.0f, -1.0f,
	0.0f,  0.0f, -1.0f,
	0.0f,  0.0f, -1.0f,
	0.0f,  0.0f, -1.0f,
	0.0f,  0.0f, 1.0f,
	0.0f,  0.0f, 1.0f,
	0.0f,  0.0f, 1.0f,
	0.0f,  0.0f, 1.0f,
	0.0f,  0.0f, 1.0f,
	0.0f,  0.0f, 1.0f,
	-1.0f,  0.0f,  0.0f,
	-1.0f,  0.0f,  0.0f,
	-1.0f,  0.0f,  0.0f,
	-1.0f,  0.0f,  0.0f,
	-1.0f,  0.0f,  0.0f,
	-1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f,
	0.0f, -1.0f,  0.0f,
	0.0f, -1.0f,  0.0f,
	0.0f, -1.0f,  0.0f,
	0.0f, -1.0f,  0.0f,
	0.0f, -1.0f,  0.0f,
	0.0f, -1.0f,  0.0f,
	0.0f,  1.0f,  0.0f,
	0.0f,  1.0f,  0.0f,
	0.0f,  1.0f,  0.0f,
	0.0f,  1.0f,  0.0f,
	0.0f,  1.0f,  0.0f,
	0.0f,  1.0f,  0.0f
};

void f3d::material_map::Prepare(glm::u32vec3 scene_size, const char* path, glm::vec4 color)
{
	_color = color;
	vector<float> positions; // positions of non-zero voxels
	vector<float> materials; // material indexes for voxels: 1.0/material_number
    
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao); // 1. bind Vertex Array Object
	glGenBuffers(4, buff);

	// vertices
	glBindBuffer(GL_ARRAY_BUFFER, buff[0]); // copy vertices array to buffer for OpenGL use
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 108, vertices, GL_STATIC_DRAW);
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
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 108, normals, GL_STATIC_DRAW);
    glVertexAttribPointer(
        1, // attribute "location = 1" in shader
        3, // size of attribute (in floats, normal vector is vec3)
        GL_FLOAT,
        GL_FALSE,
        3 * sizeof(float), // space between consecutive normal attributes
        (void*)0 // offset of this attribute in vertices "structure"
    );
    glEnableVertexAttribArray(1);

	// load from file & parse to positions
	// TODO: move this to GPU
	try
	{
		ifstream f;
		f.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		f.open(path, ios_base::binary);
		// just ignoring scene_size.z and assuming from size of file
		f.seekg(0, f.end);
		size_t length = f.tellg();
		f.seekg(0, f.beg);
		// total height (axis Z)
		scene_size.z = length / (scene_size.x * scene_size.y);
		vector<uint8_t> slice;
		slice.reserve((size_t)scene_size.x * scene_size.y);
		nr_of_instances = 0;
		for(uint32_t z = 0; z < scene_size.z; z++)
		{
			// read one slice (x, y = var.; z = const)
			f.read((char*)slice.data(), (size_t)scene_size.x * scene_size.y);
			// find non-zero voxels
			size_t idx = 0;
			for(uint32_t y = 0; y < scene_size.y; y++)
			{
				for(uint32_t x = 0; x < scene_size.x; x++)
				{
					uint8_t material = slice[idx++];
					if(material != 0)
					{
						// store non-zero voxel's coordinates
						positions.push_back((float)x);
						positions.push_back((float)y);
						positions.push_back((float)z);
						materials.push_back((float)material / 256.0f);
						nr_of_instances++;
					}
				}
			}
		}
	}
	catch(const std::exception& e)
	{
		cerr << "Can't load material (voxel-map) file: " << path;
		cerr << "\n" << e.what() << '\n';
	}

	// instances of cube - translations
	glBindBuffer(GL_ARRAY_BUFFER, buff[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * positions.size(), positions.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glVertexAttribDivisor(2, 1); // last arg: 0 = update the content of the vertex attribute each iteration of the vertex shader, 1 = update the content of the vertex attribute when we start to render a new instance, 2... = update the content every 2 instances and so on
	glEnableVertexAttribArray(2);

	// instances of cube - translations
	glBindBuffer(GL_ARRAY_BUFFER, buff[3]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * materials.size(), materials.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
	glVertexAttribDivisor(3, 1); // last arg: 0 = update the content of the vertex attribute each iteration of the vertex shader, 1 = update the content of the vertex attribute when we start to render a new instance, 2... = update the content every 2 instances and so on
	glEnableVertexAttribArray(3);
}

void f3d::material_map::Draw(const glm::mat4& view_matrix, const glm::vec3& light_pos)
{
    _shader->use();
    // Create transformationmatrix from rotation, size and position(translation)
    // than multiply with view_matrix before pass to GPU
/*    auto trans_mat = glm::mat4(1.0f);
    trans_mat = glm::translate(trans_mat, _translation);
    trans_mat = f3d::RotationMatrix(_rotation) * trans_mat;
    trans_mat = glm::scale(trans_mat, _scale);
    auto normal_mat = glm::mat3(glm::transpose(glm::inverse(trans_mat))); // transformation matrix for normals
*/
    _shader->setUniform("view", view_matrix);
//    _shader->setUniform("transform", trans_mat);
//    _shader->setUniform("normal_mat", normal_mat);
   _shader->setUniform("transform", glm::mat4(1.0f)); // transformations are already done in STL2VOX utility
   _shader->setUniform("normal_mat", glm::mat3(1.0f)); // before converting to voxel map
    _shader->setUniform("light_pos", light_pos);

    glBindVertexArray(vao);
    // glPolygonMode(GL_FRONT_AND_BACK,GL_LINE); // wireframe
    // glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 36, nr_of_instances);
}
