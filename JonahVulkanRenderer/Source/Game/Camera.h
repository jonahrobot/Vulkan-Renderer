#pragma once

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

#include <GLFW/glfw3.h>

namespace game {

	class Camera{

	public:

		Camera(GLFWwindow* window);

		void MoveCamera(GLFWwindow* window);

		glm::mat4 GetViewMatrix();

	private:

		glm::vec3 position = glm::vec3(2.0f, 2.0f, 2.0f);
		glm::vec3 front = glm::vec3(-1.0f, -1.0f, -1.0f);
		glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f);

		float speed = 0.001f;
		float sensitivity = 100.0f;

		int width;
		int height;
	};

} // namespace game
