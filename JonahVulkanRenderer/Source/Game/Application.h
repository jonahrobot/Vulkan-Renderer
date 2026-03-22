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

	float last_frame_time;
	bool held_space = false;

	bool show_another_window = false;
	bool frustum_cull = true;
	bool first_frame_complete = false;
};

} // namespace game