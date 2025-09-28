#include "Source/Renderer/Renderer.h"
#include "Source/Game/Application.h"

int main() {

	game::Application* app = new game::Application();
	GLFWwindow* window = app->Get_Window();

	// Main Application Loop
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		app->Update();
	}

	delete app;

	return 0;
}