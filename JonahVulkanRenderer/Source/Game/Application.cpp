#include "Application.h"

namespace game {


Application::Application() {
	our_renderer = new renderer::Renderer();
}

Application::~Application() {
	delete our_renderer;
}

GLFWwindow* Application::Get_Window() {
	return our_renderer->Get_Window();
}

void Application::Update() {
	//std::cout << "Test Frame" << std::endl;
	DrawFrame();
}

void Application::DrawFrame() {
	
}

} // namespace game