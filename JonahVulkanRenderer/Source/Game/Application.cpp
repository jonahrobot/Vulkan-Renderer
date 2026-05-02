#include "Application.h"
#include "../MP Loader/MP_Parser.h"
#include "Camera.h"

#include <iostream>

#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_glfw.h>
#include <ImGui/imgui_impl_vulkan.h>
#include <ImGui/L2DFileDialog.h>

namespace game {

Application::Application() {
	last_frame_time = static_cast<float>(glfwGetTime());;

	renderer = new renderer::Renderer(960,540);
	window = renderer->Get_Window();
	camera = new Camera(window);
	renderer->AddObserver(camera);

	std::string mp_file_name = "cube.mp"; // Default with cube

	if(MP::CheckValidMP("Assets/" + mp_file_name)) {
		UpdateRenderTarget("Assets/" + mp_file_name);
	};

	renderer::Renderer::DrawInfo last_draw_info = renderer->GetLightData();
	light_color[0] = last_draw_info.LightColor.x;
	light_color[1] = last_draw_info.LightColor.y;
	light_color[2] = last_draw_info.LightColor.z;

	light_position[0] = last_draw_info.LightPosition.x;
	light_position[1] = last_draw_info.LightPosition.y;
	light_position[2] = last_draw_info.LightPosition.z;

	light_mode = last_draw_info.DrawMode;

	FileDialog::file_dialog_open = false;
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

	// Start new frame
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Check if UI using input
	ImGuiIO& io = ImGui::GetIO();

	ImGuiStyle& style = ImGui::GetStyle();
	style.FontScaleMain = 1.5f;

	// Prepare UI
	static float f = 0.0f;
	static int counter = 0;
	static int draw_mode = 0;
	const char* draw_mode_options[] = { "Normals", "Soft Shading"};
	static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	static char* file_dialog_buffer = nullptr;
	static char path[500] = "";

	if (first_frame_complete == false) {
		ImGui::SetNextWindowPos(ImVec2(32.0f, 32.0f));
		ImGui::SetNextWindowSize(ImVec2(400, 500));
		first_frame_complete = true;
	}

	ImGui::Begin("Vulkan Renderer", nullptr, ImGuiWindowFlags_NoTitleBar);

	ImGui::Text("Vulkan Renderer 1.0.0");
	ImGui::SameLine(0.0,40.0f);
	if (ImGui::Button("Load MP##path")) {
		file_dialog_buffer = path;
		FileDialog::file_dialog_open = true;
		FileDialog::file_dialog_open_type = FileDialog::FileDialogType::OpenFile;
	}

	ImGui::Dummy(ImVec2(0, 20.0f)); ImGui::SeparatorText("General");

	ImGui::Text("Rendering: %s", current_file_name.c_str());

	ImGui::Dummy(ImVec2(0, 20.0f)); ImGui::SeparatorText("Lighting");

	if (ImGui::ColorEdit3("Light Color", light_color)) {
		renderer->UpdateLightColor(glm::vec3(light_color[0], light_color[1], light_color[2]));
	}

	if (ImGui::InputFloat3("Light Position", light_position)) {
		renderer->UpdateLightPosition(glm::vec3(light_position[0], light_position[1], light_position[2]));
	}

	if (ImGui::Combo("Lighting Mode", &draw_mode, draw_mode_options, IM_ARRAYSIZE(draw_mode_options))) {
		renderer->UpdateDrawMode((renderer::DRAWMODE) draw_mode);
	}
	
	ImGui::Dummy(ImVec2(0, 20.0f)); ImGui::SeparatorText("Camera");

	float current_position[3] = { camera_position.x, camera_position.y, camera_position.z };
	if (ImGui::InputFloat3("Camera Position", current_position)) {
		camera->SetPosition(glm::vec3(current_position[0], current_position[1], current_position[2]));
	}
	ImGui::Checkbox("Pause Frustum Culling", &freeze_frustum_cull);

	if (FileDialog::file_dialog_open) {
		FileDialog::ShowFileDialog(&FileDialog::file_dialog_open, file_dialog_buffer, sizeof(file_dialog_buffer), FileDialog::file_dialog_open_type);
	}

	if (file_dialog_buffer && MP::CheckValidMP(file_dialog_buffer)) {
		UpdateRenderTarget(file_dialog_buffer);

		file_dialog_buffer = nullptr;
		path[0] = '\0';	
	}

	ImGui::End();

	// Move objects
	camera->MoveCamera(window, delta_time, !io.WantCaptureKeyboard, !io.WantCaptureMouse);
	camera_position = camera->GetPosition();

	// Draw scene
	renderer->Draw(camera->GetViewMatrix(), !freeze_frustum_cull);
}

void Application::UpdateRenderTarget(std::string json_file_path) {

	// Load scene data
	std::vector<renderer::MeshInstances> model_set = MP::ParseMP(json_file_path, false);
	renderer->UpdateModelSet(model_set, true);

	// Set camera root
	glm::vec3 scene_root = renderer->GetSceneRoot();
	camera->SetPosition(scene_root);

	// Update app state
	camera_position = scene_root;
	current_file_name = MP::GetNameMP(json_file_path);
}

} // namespace game