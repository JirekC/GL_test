#include "f3d/driver_data.hpp"

f3d::driver_data::driver_data(const char* data_path) : data(data_path)
{
	data_file.close();
}
