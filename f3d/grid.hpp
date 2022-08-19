#ifndef F3D_GRID_HPP
#define F3D_GRID_HPP

#include <vector>
#include "glm/glm.hpp"
#include "shader.hpp"

namespace f3d {

    struct grid {

		shader* _shader;
		unsigned int vao; // Vertex Array Object ID
		unsigned int buff; // Vertex / Element Buffer Object ID
        size_t nr_of_vertices = 0;
        glm::vec4 color = {0.5f, 0.5f, 0.5f, 1.0f}; // default is gray
        glm::vec3 line_spacing; // in [simulation units - dx]

        grid(shader& grid_shader) { _shader = &grid_shader; }

        void Prepare(glm::vec3 scene_size, glm::vec<3, uint32_t> nr_of_lines);

        void Draw(const glm::mat4& view_matrix);

    };

}

#endif
