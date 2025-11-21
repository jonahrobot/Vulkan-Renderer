#pragma once
#include "../Renderer/Renderer.h"
#include "Camera.h"

namespace game {

class Application {

public:
	Application();
	~Application();

	GLFWwindow* Get_Window();

	void Update();

private:
	GLFWwindow* window;
	renderer::Renderer* renderer;
	Camera* camera;

	//glm::mat4 camera_position = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	void DrawFrame();

};

} // namespace game