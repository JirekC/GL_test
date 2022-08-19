#ifndef RENDER_HPP
#define RENDER_HPP

#include <iostream>
#include <vector>
#include <glm/glm.hpp> // OpenGL math (C++ wrap)
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "f3d/shader.hpp"

namespace f3d {

	struct render {
		
		shader shader;
		unsigned int vao; // Vertex Array Object ID
		unsigned int buff[4]; // Vertex / Element Buffer Object ID
		unsigned int num_instances = 0;
		glm::mat4 view;
		glm::vec4 color = { 0.0f, 0.0f, 0.0f, 1.0f };
		
		render(const char* vertex_shader_file, const char* fragment_shader_file);
		
		void setPositions(const std::vector<glm::vec4>& pos);
		void setView(const glm::mat4& view) { this->view = view; }
		void setColor(glm::vec4 clr) { color = clr; }
		void Draw();

	};

}

#endif
