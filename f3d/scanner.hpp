#ifndef SCANNER_HPP
#define SCANNER_HPP

#include <cstdint>
#include <iostream>
#include <fstream>
#include <vector>
#include "glm/glm.hpp"
#include "f3d/shader.hpp"

namespace f3d {

	struct scanner {

		// data
		glm::vec3 position;
		glm::vec3 rotation;
		glm::u32vec2 size = {0, 0}; // always 2D rectangle
		std::ifstream data_file;
		uint32_t num_frames = 0; ///< number of frames stored in data_file
		uint32_t store_every_nth_frame = 1; 
		std::vector<glm::float32> values; ///< holds values of actual frame

		// OpenGL
		shader* _shader;
		unsigned int vao; // Vertex Array Object ID
		unsigned int buff; // Vertex / Element Buffer Object ID
	    unsigned int texture; // texture of values ID

		scanner(shader& scan_shader)
		{
			_shader = &scan_shader;
		}

		scanner(shader& scan_shader,
				glm::u32vec3 position,
				glm::vec3 rotation,
				glm::u32vec2 size,
				std::string data_file_name,
				uint32_t store_every_nth_frame = 1)
		{
			_shader = &scan_shader;
			Prepare(position, rotation, size, data_file_name, store_every_nth_frame);
		}

		~scanner() {
			if (data_file.is_open())
				data_file.close();
		}
		
		void Prepare(glm::u32vec3 position, glm::vec3 rotation, glm::u32vec2 size, std::string data_file_name, uint32_t store_every_nth_frame = 1);

		/**
		* \brief	loads one frame from \ref data_file to \ref values
		* \return	0: OK; -1: frame out of range; -2: IO error
		*/
		int load_frame(uint32_t frame);

		void Draw(const glm::mat4& view_matrix);

	};

}

#endif
