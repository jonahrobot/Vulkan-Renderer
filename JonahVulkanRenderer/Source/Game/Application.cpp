#include "Application.h"

// Unnamed namespace to show functions below are pure Utility strictly for this .cpp file.
namespace {
}

namespace game {


Application::Application() {
	renderer = new renderer::Renderer();
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

	float frame_time = glfwGetTime();
	float delta_time = frame_time  - last_frame_time;
	last_frame_time = frame_time;

	camera->MoveCamera(window, delta_time);
	renderer->Draw(camera->GetViewMatrix());

	if (!held_space && glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
		renderer::detail::ModelData model_null = {};
		renderer->UpdateModelSet({ model_null });
		held_space = true;
	}

	if (held_space && glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE) {
		renderer::detail::ModelData model_0 = renderer::detail::LoadModel("models/iron_golem.obj");

		renderer->UpdateModelSet({ model_0 });
		held_space = false;
	}
}

void Application::DrawFrame() {
	
}

} // namespace game