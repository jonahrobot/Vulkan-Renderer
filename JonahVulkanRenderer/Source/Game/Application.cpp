#include "Application.h"
#include "../MP Loader/MP_Parser.h"
#include "Camera.h"

#include <iostream>

#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_glfw.h>
#include <ImGui/imgui_impl_vulkan.h>

namespace game {

Application::Application() {
	last_frame_time = static_cast<float>(glfwGetTime());;

	renderer = new renderer::Renderer(500,400);

	std::string mp_file_name;
	std::cout << "Type the name of the .mp you would like to render. Scene file must be in the Assets folder." << std::endl;
	std::cout << "File name: ";
	std::cin >> mp_file_name;

	while (MP::CheckValidMP("Assets/" + mp_file_name) == false) {
		std::cout << "Warning: File missing from Assets folder or not valid .mp file." << std::endl;
		std::cout << "Try again, file name: ";
		std::cin >> mp_file_name;
	};

	std::vector<renderer::MeshInstances> model_set = MP::ParseMP("Assets/" + mp_file_name, false);

	renderer->UpdateModelSet(model_set,true);

	std::cout << "Model set updated." << std::endl;
	window = renderer->Get_Window();
	camera = new Camera(window);
	renderer->AddObserver(camera);

	glm::vec3 scene_root = renderer->GetSceneRoot();
	camera->SetPosition(scene_root);
	std::cout << "Scene Root is: " << scene_root.x << "," << scene_root.y << "," << scene_root.z << std::endl;
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

	// Start new frame
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Check if UI using input
	ImGuiIO& io = ImGui::GetIO();

	// Prepare UI
	ImGui::ShowDemoWindow();

	// Move objects
	camera->MoveCamera(window, delta_time, !io.WantCaptureKeyboard, !io.WantCaptureMouse);

	// Draw scene
	renderer->Draw(camera->GetViewMatrix(), frustum_cull);
}

} // namespace game