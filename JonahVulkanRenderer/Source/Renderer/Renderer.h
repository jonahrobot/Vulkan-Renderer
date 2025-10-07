#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <memory>
#include <algorithm>
#include <vector>
#include <iostream>
#include <string>

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

	GLFWwindow* Get_Window();

private:

	const std::vector<const char*> ValidationLayersToSupport = {
		"VK_LAYER_KHRONOS_validation"
	};

	const std::vector<const char*> DeviceExtensionsToSupport = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	VkInstance vulkan_instance;
	VkSurfaceKHR vulkan_surface;
	VkSwapchainKHR swapchain;
	VkPhysicalDevice physical_device;
	VkDevice logical_device;
	GLFWwindow* window;
	std::vector<VkImage> swapchain_images;
	std::vector<VkImageView> swapchain_image_views;
	VkQueue graphics_queue;
	VkQueue present_queue;
	VkPipelineLayout graphics_pipeline_layout;
};
} // namespace renderer