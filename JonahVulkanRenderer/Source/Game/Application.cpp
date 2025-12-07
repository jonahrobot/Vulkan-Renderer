#include "Application.h"

// Unnamed namespace to show functions below are pure Utility strictly for this .cpp file.
namespace {
}

namespace game {


Application::Application() {
	renderer = new renderer::Renderer();
	renderer::detail::ModelData model_0 = renderer::detail::LoadModel("models/iron_golem.obj","textures/iron_golem.png");
	renderer::detail::ModelData model_1 = renderer::detail::LoadModel("models/viking_room.obj","textures/viking_room.png");
	renderer::detail::ModelData model_2 = renderer::detail::LoadModel("models/container.obj", "textures/container.png");
	renderer->UpdateModelSet({ model_0, model_1, model_2, model_2, model_2 });
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

	if (!held_space && glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
		renderer::detail::ModelData model_null = {};
		held_space = true;
	}

	if (held_space && glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE) {
		renderer::detail::ModelData model_0 = renderer::detail::LoadModel("models/iron_golem.obj", "textures/viking_room.png");
		renderer::detail::ModelData model_1 = renderer::detail::LoadModel("models/viking_room.obj", "textures/rock.jpg");

		renderer->UpdateModelSet({ model_0, model_1 });
		held_space = false;
	}
}

void Application::DrawFrame() {
	
}

} // namespace game