#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <memory>
#include <algorithm>
#include <vector>
#include <iostream>

namespace renderer {

#define WIDTH 500
#define HEIGHT 500


class Renderer {
public:
	Renderer();
	~Renderer();

	void Draw();

private:

	const std::vector<const char*> ValidationLayers = {
		"VK_LAYER_KHRONOS_validation"
	};

	VkInstance vulkan_instance;
	VkSwapchainKHR swap_chain;
	VkPhysicalDevice physical_device;
	VkDevice logical_device;
	GLFWwindow* window;
};
} // namespace renderer