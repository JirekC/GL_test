#include "f3d/data.hpp"

f3d::data::data(const char* data_path) {

	data_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		scene_size = glm::vec3(0.0f, 0.0f, 0.0f);

		data_file.open(data_path, std::ios::binary);
		uint64_t tmp64;
		data_file.read((char*)&tmp64, 8); // number of elements
		num_elements = (unsigned int)tmp64;
		positions.resize(num_elements);
		// positions of elements
		for (unsigned int i = 0; i < num_elements; i++) {
			uint32_t tmp32;
			data_file.read((char*)&tmp32, 4);
			float tmp = (float)tmp32;
			positions[i].x = tmp;
			if (tmp > scene_size.x) scene_size.x = tmp;
		}
		for (unsigned int i = 0; i < num_elements; i++) {
			uint32_t tmp32;
			data_file.read((char*)&tmp32, 4);
			float tmp = (float)tmp32;
			positions[i].y = tmp;
			if (tmp > scene_size.y) scene_size.y = tmp;
		}
		for (unsigned int i = 0; i < num_elements; i++) {
			uint32_t tmp32;
			data_file.read((char*)&tmp32, 4);
			float tmp = (float)tmp32;
			positions[i].z = tmp;
			if (tmp > scene_size.z) scene_size.z = tmp;
			positions[i].w = 1.0f;
		}
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "ERROR::DATA::FILE_NOT_SUCCESFULLY_READ: " << data_path << ": " << e.what() << std::endl;
	}
}

glm::vec3 f3d::data::getSceneCenter() {

	glm::vec3 center = scene_size;
	center.x *= 0.5;
	center.y *= 0.5;
	center.z *= 0.5;

	return center;
}
