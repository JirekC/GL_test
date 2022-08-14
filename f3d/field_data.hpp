#pragma once

#include "f3d/data.hpp"

namespace f3d {

	struct field_data : data {
		size_t first_data_byte; ///< position of first data byte in data_file
		unsigned int num_frames = 0; ///< number of frames stored in data_file
		std::vector<glm::float32> values; ///< holds values of actual frame

		field_data(const char* data_path);
		~field_data() { data_file.close(); }
		
		/**
		* \brief	loads one frame from \ref data_file to \ref values
		* \return	0: OK; -1: frame out of range; -2: IO error
		*/
		int load_frame(unsigned int frame);
	};

}

