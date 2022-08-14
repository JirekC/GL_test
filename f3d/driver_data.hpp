#pragma once

#include "f3d/data.hpp"

namespace f3d {

	struct driver_data : data {
		std::vector<glm::float32> values;

		driver_data(const char* data_path);
	};

}
