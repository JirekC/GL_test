#include "f3d/field_data.hpp"

f3d::field_data::field_data(const char* data_path) :
	data(data_path) // load positions of elements and calc scene size
{
	try
	{
		// values
		first_data_byte = data_file.tellg();
		data_file.seekg(0, data_file.end);
		size_t length = data_file.tellg();
		length -= first_data_byte;
		data_file.seekg(first_data_byte, data_file.beg);
		//values.resize(length / sizeof(float));
		//data_file.read((char*)values.data(), 4 * (length / 4));
		//data_file.close();
		// total number of frames
		num_frames = length / (sizeof(float) * num_elements);
		values.resize(sizeof(float) * num_elements); // size of one frame in Bytes
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "ERROR::FIELD_DATA::FILE_NOT_SUCCESFULLY_READ: " << data_path << ": " << e.what() << std::endl;
	}
}

int f3d::field_data::load_frame(unsigned int frame)
{
	int ret_val = 0;
	
	if(frame < num_frames)
	{
		try
		{
			size_t position = first_data_byte + (sizeof(float) * num_elements * frame);
			data_file.seekg(position, data_file.beg);
			data_file.read((char*)values.data(), sizeof(float) * num_elements); // read actual frame
		}
		catch(std::ifstream::failure e)
		{
			std::cout << "ERROR::FIELD_DATA::FILE_DATA_READ:" << e.what() << std::endl;
			ret_val = -2; // IO error
		}
	}
	else
	{
		ret_val = -1; // out of range
	}
	
	return ret_val;
}
