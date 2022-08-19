#ifndef F3D_OBJECT_CREATOR_HPP
#define F3D_OBJECT_CREATOR_HPP

#include <vector>
#include "glm/glm.hpp"
#include "shader.hpp"

namespace f3d {

    // 3D object with vertices and indices
    struct object_3d_vi
    {
        std::vector<float> vertices; // format: {x, y, z} for each vertice
        std::vector<float> normals; // normal vector for each vertice, format: {x, y, z}
        std::vector<unsigned int> indices; // format: 3 consecutive vertices per triangle
    };
    
    namespace loader {

        object_3d_vi* LoadSTL(const char* path);

    }

    struct object3d
    {

        // data
		glm::vec3 position;
		glm::vec3 rotation;
		glm::vec3 size;
        glm::vec4 color;

		// OpenGL
		shader* _shader;
		unsigned int vao; // Vertex Array Object ID
		unsigned int buff[3]; // vertices, normals, indices (EBO)
	    size_t nr_of_vertices = 0; // total nr of vertices in final model

		object3d(shader& object_shader) { _shader = &object_shader; }

		object3d(   shader& object_shader,
                    object_3d_vi* model,
                    glm::u32vec3 position = {0, 0, 0},
                    glm::vec3 rotation = {0.0f, 0.0f, 0.0f},
                    glm::u32vec3 size = {1, 1, 1},
                    glm::vec4 color = {0.8f, 0.8f, 0.8f, 1.0f})
		{
            _shader = &object_shader;
			Prepare(model, position, rotation, size, color);
		}

		void Prepare(object_3d_vi* model,
                    glm::u32vec3 position,
                    glm::vec3 rotation,
                    glm::u32vec3 size,
                    glm::vec4 color = {0.8f, 0.8f, 0.8f, 1.0f});

        void Draw(const glm::mat4& view_matrix);

    };
    
}

#endif
