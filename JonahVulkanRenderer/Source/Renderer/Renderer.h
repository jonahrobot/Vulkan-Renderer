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

#include "../Renderer/Detail/RendererDetail.h"

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

	bool framebuffer_resized = false;

private:

	const int MAX_FRAMES_IN_FLIGHT = 2;

	const std::vector<const char*> ValidationLayersToSupport = {
		"VK_LAYER_KHRONOS_validation"
	};

	const std::vector<const char*> DeviceExtensionsToSupport = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	VkInstance vulkan_instance;
	VkSurfaceKHR vulkan_surface;
	VkSwapchainKHR swapchain;
	detail::SwapchainContext swapchain_creation_data;
	VkExtent2D extent;
	VkPhysicalDevice physical_device;
	VkDevice logical_device;
	GLFWwindow* window;
	std::vector<VkImage> swapchain_images;
	std::vector<VkImageView> swapchain_image_views;
	VkQueue graphics_queue;
	VkQueue present_queue;
	VkRenderPass render_pass;
	VkPipeline graphics_pipeline;
	VkPipelineLayout graphics_pipeline_layout;
	std::vector<VkFramebuffer> framebuffers;
	VkCommandPool command_pool;
	std::vector<VkCommandBuffer> command_buffers;
	
	std::vector<VkSemaphore> image_available_semaphores;
	std::vector<VkSemaphore> render_finished_semaphores;
	std::vector<VkFence> in_flight_fences;
	
	uint32_t current_frame = 0;

	void RecreateSwapchainHelper();
};
} // namespace renderer