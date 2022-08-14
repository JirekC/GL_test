#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include <cmath>
#include <stdint.h>
#include <glad/glad.h> // OpenGL loader
#include <GLFW/glfw3.h> // OpenGL window & input
#include <glm/glm.hpp> // OpenGL math (C++ wrap)
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "f3d/shader.hpp"
#include "f3d/field_render.hpp"
#include "f3d/field_data.hpp"
#include "f3d/object_render.hpp"


#ifndef M_PI
	#define M_PI 3.14159265358979323846264338327950288
#endif /* M_PI */

constexpr float frame_time = 20.0e-3f; // 1 / desired_fps
constexpr float rotation_sensitivity = 0.005f; // TODO: let user to adjust
float lastX, lastY;
float cam_yaw = 0.0f;
float cam_pitch = 0.0f; //was 3.14f * 0.25f;
unsigned int frame = 0;
unsigned int num_frames;

// separate thread: waiting for commands via std input
std::string cmd_line;
std::atomic<int> cmd_flag { 0 };
void cmd_input_thread(void)
{
	std::string s;
	while(std::getline(std::cin, s, '\n'))
	{
		cmd_line = std::move(s);
		if (cmd_line == "quit") {
			break;
		}
		cmd_flag = 1; // mark new command available
		while(cmd_flag > 0)
		{
			// wait for main code to read command and reset flag
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}
	cmd_flag = -1; // mark end-of-work
}

// processing inputs (from main loop)
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

// window resize callback
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// mouse move - rotation callback
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	static bool first_round = true;
	if (first_round)
	{
		lastX = xpos;
		lastY = ypos;
		first_round = false;
	}
	
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed: y ranges bottom to top
	lastX = xpos;
	lastY = ypos;
	
	cam_yaw += xoffset * rotation_sensitivity;
	cam_pitch += yoffset * rotation_sensitivity;
	if (cam_pitch >= 0.49f * M_PI) cam_pitch = 0.49f * M_PI;
	if (cam_pitch <= -0.49f * M_PI) cam_pitch = -0.49f * M_PI;
}

// mouse scroll - frame inc / dec
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	int inc = (int)yoffset * (1 + num_frames / 100);
	
	if ((int)frame + inc >= 0)
		frame += inc;
}

// main entry :)
int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(1024, 768, "GL Wave Explorer", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glViewport(0, 0, 1024, 768);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	glEnable(GL_PROGRAM_POINT_SIZE); // vertex shader can control point size
	glEnable(GL_DEPTH_TEST); // using z-buffer

	f3d::field_data data("../FAS_test/data0"); // TODO: from cmd line
	f3d::field_render field_render("f3d/vertex_field.glsl", "f3d/fragment.glsl");;
	field_render.setPositions(data.positions);
	
	// adjust move / frame-inc / rotate speed
	auto sc_size = data.getSceneSize();
	float scene_max_dim = std::max(std::max(sc_size.x, sc_size.y), sc_size.z); // maximal dimmension of scene
	float cameraSpeed = 0.35f * scene_max_dim; // adjust accordingly to scene size TODO: let user to adjust
	float rotationSpeed = M_PI * 0.5f; // 6.28 rad per 4 sec (by arrows on keyboard)
	num_frames = data.num_frames;

	f3d::data drv_elements("../FAS_test/driver"); // TODO: from cmd line
	f3d::object_render object_render("f3d/vertex_object.glsl", "f3d/fragment.glsl");
	object_render.setPositions(drv_elements.positions);
	object_render.setColor({ 1.0f,0.0f,0.0f,1.0f }); // red

	int nbFrames = 0;
	float lastTime = 0.0f;
	float last_time = 0.0f;

	glm::vec3 cam_pos = glm::vec3(0.0f, 0.0f, 3.0f);
	glm::vec3 cam_front = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 cam_up = glm::vec3(0.0f, 1.0f, 0.0f);
	unsigned int last_frame = 1;
	
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // "catch" mouse at center of window
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	
	auto io_thread = std::thread(cmd_input_thread);

	while (!glfwWindowShouldClose(window))
	{
		
		if (cmd_flag > 0)
		{
			std::cout << cmd_line << std::endl; // TODO: parse & do command
			cmd_flag = 0;
		}
		else if (cmd_flag < 0)
		{
			// "quit" string or EOF entered to std input
			break;
		}
			
		// animation - camera
		float begin_of_frame_time = glfwGetTime();
		float delta_time = begin_of_frame_time - last_time;
		last_time = begin_of_frame_time;
		
		// inputs response
		processInput(window);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// rotate
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
			cam_yaw -= delta_time * rotationSpeed;
		}
		else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
			cam_yaw += delta_time * rotationSpeed;
		}
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
			cam_pitch += delta_time * rotationSpeed;
			if (cam_pitch >= 0.49f * M_PI) cam_pitch = 0.49f * M_PI;
		}
		else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
			cam_pitch -= delta_time * rotationSpeed;
			if (cam_pitch <= -0.49f * M_PI) cam_pitch = -0.49f * M_PI;
		}
		// move
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			cam_pos += cameraSpeed * cam_front * delta_time;
		}
		else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			cam_pos -= cameraSpeed * cam_front * delta_time;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			cam_pos -= glm::normalize(glm::cross(cam_front, cam_up)) * cameraSpeed * delta_time;
		}
		else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			cam_pos += glm::normalize(glm::cross(cam_front, cam_up)) * cameraSpeed * delta_time;
		}
		if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
			cam_pos += glm::normalize(glm::cross(glm::normalize(glm::cross(cam_front, cam_up)), cam_front)) * cameraSpeed * delta_time;
		}
		else if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
			cam_pos -= glm::normalize(glm::cross(glm::normalize(glm::cross(cam_front, cam_up)), cam_front)) * cameraSpeed * delta_time;
		}
		// wave animation - slow motion
		if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) {
			if (frame > 0) frame--;
		}
		else if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS) {
			//if (frame < data.num_frames - 1) frame++;
			frame++;
		}
		// wave animation - fast motion
		if (glfwGetKey(window, GLFW_KEY_END) == GLFW_PRESS) {
			if (frame >= (1 + num_frames / 100))
				frame -= (1 + num_frames / 100);
		}
		else if (glfwGetKey(window, GLFW_KEY_HOME) == GLFW_PRESS) {
			frame += (1 + num_frames / 100);
		}

		glm::vec3 direction;
		direction.x = cos(cam_yaw) * cos(cam_pitch);
		direction.y = sin(cam_pitch);
		direction.z = sin(cam_yaw) * cos(cam_pitch);
		cam_front = glm::normalize(direction);

		glm::mat4 camera = glm::lookAt(cam_pos, cam_pos + cam_front, cam_up);
		glm::ivec4 viewport;
		glGetIntegerv(GL_VIEWPORT, &viewport.x); // get viewport position and size
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)viewport.z / viewport.w, 1.0f, 2.0f * scene_max_dim);
		camera = projection * camera;
		field_render.setView(camera);
		object_render.setView(camera);
		
		if (last_frame != frame)
		{
			// not same frame, load new values from file
			field_render.setValues(data, frame);
			last_frame = frame;
		}

		// render
		field_render.Draw();
		object_render.Draw();

		// check and call events and swap the buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
		
		// FPS limiter - power save
		float now = glfwGetTime();
		if (now - begin_of_frame_time < frame_time) {
			std::this_thread::sleep_for(std::chrono::milliseconds((int)(1e3f * (frame_time - now + begin_of_frame_time))));
		}

		// some usefull info
		// TODO: move it to gfx window
		printf("FPS: %f\n\r", 1.0f / (glfwGetTime() - begin_of_frame_time));
		printf("Frame: %u \033[F", frame);
	}

	glfwTerminate();
	std::exit(0); // io_thread will be forced to end here
	return 0;
}
