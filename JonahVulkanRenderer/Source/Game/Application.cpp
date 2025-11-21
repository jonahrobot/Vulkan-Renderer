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
	camera->MoveCamera(window);
	renderer->Draw(camera->GetViewMatrix());
}

void Application::DrawFrame() {
	
}

} // namespace game