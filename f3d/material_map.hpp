#ifndef MATERIAL_MAP_HPP
#define MATERIAL_MAP_HPP

#include <vector>
#include "glm/glm.hpp"
#include "shader.hpp"

namespace f3d {

    struct material_map
    {
        shader* _shader;
        unsigned int vao; // Vertex Array Object ID
		unsigned int buff[3]; // vertices, normals, positions (instances)
        glm::vec4 _color;
        size_t nr_of_instances = 0;

        material_map(shader& mat_shader) { _shader = &mat_shader; }

        material_map(shader& mat_shader,
                    glm::u32vec3 scene_size,
                    const char* path,
                    glm::vec4 color = {0.8f, 0.8f, 0.8f, 1.0f})
        {
            _shader = &mat_shader;
            Prepare(scene_size, path, color);
        }

        void Prepare(
                    glm::u32vec3 scene_size,
                    const char* path,
                    glm::vec4 color = {0.8f, 0.8f, 0.8f, 1.0f});

        void Draw(const glm::mat4& view_matrix, const glm::vec3& light_pos);
    };

}

#endif
