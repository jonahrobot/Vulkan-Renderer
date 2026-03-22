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

	renderer = new renderer::Renderer(960,540);

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
	static float f = 0.0f;
	static int counter = 0;
	static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	if (first_frame_complete == false) {
		ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
		ImGui::SetNextWindowSize(ImVec2(400, 500));
		first_frame_complete = true;
	}

	ImGui::Begin("Vulkan Renderer", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

	ImGui::Text("Vulkan Renderer 1.0.0");

	ImGui::SeparatorText("Lighting");

	
	ImGui::SeparatorText("Camera");

	//ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
	//ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

	//if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
	//	counter++;
	//ImGui::SameLine();
	//ImVec2 size = ImGui::GetWindowSize();
	//float width = size.x;
	//float height = size.y;
	//ImGui::Text("Screen size is = %f by %f", width, height);

	//ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
	ImGui::End();

	// Move objects
	camera->MoveCamera(window, delta_time, !io.WantCaptureKeyboard, !io.WantCaptureMouse);

	// Draw scene
	renderer->Draw(camera->GetViewMatrix(), frustum_cull);
}

} // namespace game