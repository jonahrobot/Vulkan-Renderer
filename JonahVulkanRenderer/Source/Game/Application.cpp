#include "Application.h"
#include "../MP Loader/MP_Parser.h"

#include <iostream>

namespace game {

Application::Application() {
	last_frame_time = static_cast<float>(glfwGetTime());;

	renderer = new renderer::Renderer();

	std::vector<renderer::detail::MeshInstances> model_set = MP::ParseMP("ExternalTools/dev.mp", false);

	renderer->UpdateModelSet(model_set,true);

	std::cout << "Model set updated." << std::endl;
	window = renderer->Get_Window();
	camera = new Camera(window);
}

Application::~Application() {
	delete renderer;
	delete camera;
}

GLFWwindow* Application::Get_Window() {
	return window;
}

void Application::Update() {

	if(glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
		if (frustum_cull) {
			frustum_cull = false;
		}
	}
	else {
		if (frustum_cull == false) {
			frustum_cull = true;
		}
	}

	float frame_time = static_cast<float>(glfwGetTime());
	float delta_time = frame_time  - last_frame_time;
	last_frame_time = frame_time;

	camera->MoveCamera(window, delta_time);
	renderer->Draw(camera->GetViewMatrix(), frustum_cull);

}

} // namespace game