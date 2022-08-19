#include "f3d/scanner.hpp"
#include "f3d/f3d_math.hpp"

// 3D objet - rectangle
static float vertices[] = {
	// first triangle
	// 3D coord                 // texture coords
	1.0f, 1.0f, 0.0f, 1.0f,     1.0f, 1.0f,	        // top right
	1.0f, 0.0f, 0.0f, 1.0f,     1.0f, 0.0f,         // bottom right
	0.0f, 1.0f, 0.0f, 1.0f,     0.0f, 1.0f,         // top left
	// second triangle
	1.0f, 0.0f, 0.0f, 1.0f,     1.0f, 0.0f,         // bottom right
	0.0f, 0.0f, 0.0f, 1.0f,     0.0f, 0.0f,         // bottom left
	0.0f, 1.0f, 0.0f, 1.0f,     0.0f, 1.0f,         // top left
};

void f3d::scanner::Prepare(glm::u32vec3 position, glm::vec3 rotation, glm::u32vec2 size, std::string data_file_name, uint32_t store_every_nth_frame)
{
    this->size = size;
    this->rotation = rotation;
    this->position = position;
    this->store_every_nth_frame = store_every_nth_frame;

	/*** Data part ***/
	try
	{
		data_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		data_file.open(data_file_name, std::ios::binary);
		// values
		data_file.seekg(0, data_file.end);
		size_t length = data_file.tellg();
		data_file.seekg(0, data_file.beg);
		// total number of frames
		num_frames = length / (sizeof(float) * size.x * size.y);
		values.resize((size_t)(size.x) * size.y, 0.0f); // size of one frame in Bytes
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "ERROR::f3d::scanner::Prepare(): FILE_NOT_SUCCESFULLY_READ: " << data_file_name << ":\n" << e.what() << std::endl;
        return;
	}

	/*** OpenGL part ***/
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao); // bind Vertex Array Object
	glGenBuffers(1, &buff);
	// vertices
	glBindBuffer(GL_ARRAY_BUFFER, buff); // copy vertices array to buffer for OpenGL use
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // vertices - 3D coordinates
    glVertexAttribPointer(
        0, // attribute "location = 0" in shader
        4, // size of attribute (in floats, vertex coord is vec4)
        GL_FLOAT,
        GL_FALSE,
        6 * sizeof(float), // space between consecutive vertex attributes
        (void*)0 // offset of this attribute in vertices "structure"
    );
    glEnableVertexAttribArray(0);
    // vertices - texture coordinates
    glVertexAttribPointer(
        1, // attribute "location = 1" in shader
        2, // size of attribute (in floats, vertex coord is vec4)
        GL_FLOAT,
        GL_FALSE,
        6 * sizeof(float), // space between consecutive vertex attributes
        (void*)(4 * sizeof(float)) // offset of this attribute in vertices "structure"
    );
    glEnableVertexAttribArray(1);
    // texture of values
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping/filtering options (on currently bound texture)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // frame 0
    glTexImage2D(
        GL_TEXTURE_2D,
        0, // level of mipmaps (0 = not used)
        GL_RED, // internal format of texture in GPU (single channel)
        size.x, size.y,
        0,
        GL_RED, // format of data
        GL_FLOAT, // type of data
        values.data() // all zeroes from beginning
    );

}

int f3d::scanner::load_frame(uint32_t frame)
{
	int ret_val = 0;

    frame = frame / store_every_nth_frame; // scale from simulation-frames to data-file-frames
	
	if(frame < num_frames)
	{
		try
		{
			size_t position = sizeof(float) * size.x * size.y * frame;
			data_file.seekg(position, data_file.beg);
			data_file.read((char*)values.data(), sizeof(float) * values.size()); // read actual frame
            // update texture - glTexSubImage2D() is faster than creating new texture by glTexImage2D()
            glBindTexture(GL_TEXTURE_2D, texture);     
            glTexSubImage2D(
                GL_TEXTURE_2D,
                0, // level of mipmaps (0 = not used)
                0, 0, // offset on original texture (0, 0)
                size.x, size.y,
                GL_RED, // format of data
                GL_FLOAT, // type of data
                values.data()
            );
		}
		catch(std::ifstream::failure e)
		{
			std::cout << "ERROR::FIELD_DATA::FILE_DATA_READ:" << e.what() << std::endl;
			ret_val = -2; // IO error
		}
	}
	else
	{
		ret_val = -1; // out of range
	}
	
	return ret_val;
}

void f3d::scanner::Draw(const glm::mat4& view_matrix)
{
    _shader->use();
    // Create transformationmatrix from rotation, size and position(translation)
    // than multiply with view_matrix before pass to GPU
    glm::mat4 trans = glm::mat4(1.0f);
    trans = glm::translate(trans, position);
    trans = f3d::RotationMatrix(rotation) * trans;
    trans = glm::scale(trans, {(float)(size.x), (float)(size.y), 1.0f});
    _shader->setUniform("view", view_matrix * trans);

    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
