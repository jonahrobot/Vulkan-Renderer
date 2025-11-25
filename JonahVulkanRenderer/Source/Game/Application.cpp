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
		std::vector<renderer::detail::Vertex> vertices_to_render;
		std::vector<uint32_t> indices;
		renderer->UpdateDrawVertices(vertices_to_render,indices);
		held_space = true;
	}

	if (held_space && glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE) {
		renderer::detail::ModelData model_0 = renderer::detail::LoadModel("models/viking_room.obj");

		renderer::detail::MergedIndexVertexBuffer merged_data{};
		merged_data = renderer::detail::MergeIndexVertexBuffer(model_0.vertices_to_render, model_0.indices, model_0.vertices_to_render, model_0.indices);

		renderer->UpdateDrawVertices(merged_data.merged_vertex_buffer, merged_data.merged_index_buffer);
		held_space = false;
	}
}

void Application::DrawFrame() {
	
}

} // namespace game