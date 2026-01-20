#include "Application.h"
#include "../USD Loader/USDParser.h"

// Unnamed namespace to show functions below are pure Utility strictly for this .cpp file.
namespace {
}

namespace game {


Application::Application() {
	renderer = new renderer::Renderer();

	std::vector<renderer::detail::InstanceModelData> model_set = USD::ParseUSD("ExternalTools/dev.mp", false);

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

	float frame_time = static_cast<float>(glfwGetTime());
	float delta_time = frame_time  - last_frame_time;
	last_frame_time = frame_time;

	camera->MoveCamera(window, delta_time);
	renderer->Draw(camera->GetViewMatrix());

}

} // namespace game