#pragma once
#include "f3d/render.hpp"

namespace f3d {

	struct object_render : render {
		
		object_render(const char* vertex_shader_file, const char* fragment_shader_file) : render(vertex_shader_file, fragment_shader_file) {};
		
	};

}
