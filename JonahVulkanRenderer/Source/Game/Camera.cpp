#include "Camera.h"
#include <iostream>

namespace game {

	Camera::Camera(GLFWwindow* window) {
		glfwGetFramebufferSize(window, &width, &height);
	}

	void Camera::MoveCamera(GLFWwindow* window) {

		// XY Movement
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			position += speed * front;
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			position += speed * -front;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			position += speed * -glm::normalize(glm::cross(front,up));
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			position += speed * glm::normalize(glm::cross(front, up));
		}

		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
			speed = 0.005f;
		}
		else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE) {
			speed = 0.001f;
		}

		// Z Movement
		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
			position += speed * up;
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) {
			position += speed * -up;
		}

		// Mouse look
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

			double mouse_x;
			double mouse_y;
			glfwGetCursorPos(window, &mouse_x, &mouse_y);
			
			float rotation_x = sensitivity * (float)(mouse_y - (height / 2)) / height;
			float rotation_y = sensitivity * (float)(mouse_x - (width / 2)) / height;

			glm::vec3 new_orientation = glm::rotate(front, glm::radians(-rotation_x), glm::normalize(glm::cross(front, up)));

			if (!(glm::angle(new_orientation, up) <= glm::radians(5.0f) || glm::angle(new_orientation, -up) <= glm::radians(5.0f))) {
				front = new_orientation;
			}

			front = glm::rotate(front, glm::radians(-rotation_y), up);

			glfwSetCursorPos(window, (width / 2), (height / 2));
		}
	}

	glm::mat4 Camera::GetViewMatrix() {
		//return glm::lookAt(position, position + orientation, up);
		return glm::lookAt(position, position + front, up);
	}

}