#include "Camera.h"
#include <iostream>

namespace game {

	Camera::Camera(GLFWwindow* window) {
		glfwGetFramebufferSize(window, &render_width, &render_height);
	}

	void Camera::MoveCamera(GLFWwindow* window, float delta_time) {

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			position += speed * front * delta_time;
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			position += speed * -front * delta_time;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			position += speed * -glm::normalize(glm::cross(front,up)) * delta_time;
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			position += speed * glm::normalize(glm::cross(front, up)) * delta_time;
		}

		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
			speed = 5.0f;
		}
		else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE) {
			speed = 1.0f;
		}
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
			position += speed * up * delta_time;
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) {
			position += speed * -up * delta_time;
		}
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {

			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

			if (first_click_flag) {
				glfwSetCursorPos(window, (render_width / 2), (render_height / 2));
				first_click_flag = false;
			}
			
			double mouse_x;
			double mouse_y;
			glfwGetCursorPos(window, &mouse_x, &mouse_y);
			
			float rotation_x = sensitivity * (float)(mouse_y - (render_height / 2)) / render_height;
			float rotation_y = sensitivity * (float)(mouse_x - (render_width / 2)) / render_width;

			glm::vec3 new_orientation = glm::rotate(front, glm::radians(-rotation_x), glm::normalize(glm::cross(front, up)));

			if (!(glm::angle(new_orientation, up) <= glm::radians(5.0f) || glm::angle(new_orientation, -up) <= glm::radians(5.0f))) {
				front = new_orientation;
			}

			front = glm::rotate(front, glm::radians(-rotation_y), up);

			glfwSetCursorPos(window, (render_width / 2), (render_height / 2));
		}
		else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			first_click_flag = true;
		}
	}

	glm::mat4 Camera::GetViewMatrix() {
		return glm::lookAt(position, position + front, up);
	}

}