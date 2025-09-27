#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <memory>
#include <algorithm>
#include <vector>
#include <iostream>

#ifdef NDEBUG
const bool UseValidationLayers = false;
#else
const bool UseValidationLayers = true;
#endif

namespace renderer {

#define WIDTH 500
#define HEIGHT 500

class Renderer {
public:
	Renderer();
	~Renderer();

	void Draw();

private:

	const std::vector<const char*> ValidationLayersToSupport = {
		"VK_LAYER_KHRONOS_validation"
	};

	const std::vector<const char*> DeviceExtensionsToSupport = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	VkInstance vulkan_instance;
	VkSurfaceKHR vulkan_surface;
	VkSwapchainKHR swap_chain;
	VkPhysicalDevice physical_device;
	VkDevice logical_device;
	GLFWwindow* window;
};
} // namespace renderer