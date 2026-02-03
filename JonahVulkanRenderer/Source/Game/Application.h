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
	renderer::Renderer* renderer;
	GLFWwindow* window;
	Camera* camera;

	float last_frame_time;
	bool held_space = false;
	bool frustum_cull = true;
};

} // namespace game