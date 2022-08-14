
#include "f3d/field_render.hpp"

void f3d::field_render::setValues(field_data& fd, unsigned int frame)
{
	// instances of cube - colors / values
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, buff[3]);
	if (fd.load_frame(frame) == 0)
	{
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::float32) * num_instances, fd.values.data(), GL_DYNAMIC_DRAW);
	}
	else
	{
		// frame is out-of-range or general IO error -> force to zero
		glm::float32* dummy = new glm::float32[num_instances];
		std::fill_n(dummy, num_instances, 0.0f);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::float32) * num_instances, dummy, GL_DYNAMIC_DRAW);
		delete[] dummy;
	}
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0 * sizeof(float), (void*)0);
	glVertexAttribDivisor(2, 1); // last arg: 0 = update the content of the vertex attribute each iteration of the vertex shader, 1 = update the content of the vertex attribute when we start to render a new instance of model, 2... = update the content every 2 instances and so on
	glEnableVertexAttribArray(2);
}
