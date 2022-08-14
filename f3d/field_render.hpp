#pragma once

#include "f3d/render.hpp"
#include "f3d/field_data.hpp"

namespace f3d {

	struct field_render : render {
		field_render(const char* vertex_shader_file, const char* fragment_shader_file) : render(vertex_shader_file, fragment_shader_file) {};

		void setValues(field_data& fd, unsigned int frame);
	};
}
