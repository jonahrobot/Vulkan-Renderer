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
	bool freeze_frustum_cull = false;
	bool first_frame_complete = false;
	glm::vec3 camera_position;

	float light_color[3];
	float light_position[3];
	int light_mode;

	std::string current_file_name;

	void UpdateRenderTarget(std::string json_file_path);
};

} // namespace game