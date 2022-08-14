#include "f3d/camera.hpp"

glm::mat4 f3d::camera::getViewMatrix() {
	if (!actual) {
		// recalc
		/*glm::mat4 camera = glm::lookAt(glm::vec3(camX, camY, camZ),  // position of camera
			cam_pos, //glm::vec3(0.0f, 0.0f, 0.0f),  // center of scene - target
			glm::vec3(0.0f, 1.0f, 0.0f)); // up vector
		glm::ivec4 viewport;
		glGetIntegerv(GL_VIEWPORT, &viewport.x); // get viewport position and size
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)viewport.z / viewport.w, 0.1f, 400.0f);
		view_matrix = projection * camera;
		actual = true;*/
	}

	return view_matrix;
}
