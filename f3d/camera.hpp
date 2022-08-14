#pragma once

#include <iostream>
#include <vector>
#include <glm/glm.hpp> // OpenGL math (C++ wrap)
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "f3d/shader.hpp"

namespace f3d {

	struct camera {
		bool actual = false;
		glm::mat4 view_matrix;

		glm::mat4 getViewMatrix();
	};

}
