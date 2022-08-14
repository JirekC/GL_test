#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <stdint.h>
#include <glm/glm.hpp> // OpenGL math (C++ wrap)
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace f3d {

	struct data {
	protected:
		std::ifstream data_file;
	public:
		unsigned int num_elements = 0;
		glm::vec3 scene_size;
		std::vector<glm::vec4> positions;

		data(const char* data_path);

		glm::vec3 getSceneSize() { return scene_size; }
		glm::vec3 getSceneCenter();
	};

}
