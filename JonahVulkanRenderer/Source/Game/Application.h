#pragma once
#include "../Renderer/Renderer.h"

namespace game {

class Application {

public:
	Application();
	~Application();

	GLFWwindow* Get_Window();

	void Update();

private:
	renderer::Renderer* our_renderer;

	void DrawFrame();

};

} // namespace game