#include "Application.h"

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
}

void Application::DrawFrame() {
	
}

} // namespace game