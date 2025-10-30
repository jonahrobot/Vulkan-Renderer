#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <memory>
#include <algorithm>
#include <vector>
#include <iostream>
#include <glm/glm.hpp>
#include <array>
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

	struct Vertex {
		glm::vec2 position;
		glm::vec3 color;

		static VkVertexInputBindingDescription GetBindingDescription() {
			VkVertexInputBindingDescription binding_description{};
			binding_description.binding = 0;
			binding_description.stride = sizeof(Vertex);
			binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			return binding_description;
		}

		static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescription() {
			std::array<VkVertexInputAttributeDescription, 2> attribute_descriptions = {};

			// Position
			attribute_descriptions[0].binding = 0;
			attribute_descriptions[0].location = 0;
			attribute_descriptions[0].format = VK_FORMAT_R32G32_SFLOAT; // Vec2
			attribute_descriptions[0].offset = offsetof(Vertex, position);

			// Color
			attribute_descriptions[1].binding = 0;
			attribute_descriptions[1].location = 1;
			attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT; // Vec3
			attribute_descriptions[1].offset = offsetof(Vertex, color);

			return attribute_descriptions;
		}
	};

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
	VkBuffer vertex_buffer;
	VkDeviceMemory vertex_buffer_memory;
	std::vector<VkCommandBuffer> command_buffers;
	
	std::vector<VkSemaphore> image_available_semaphores;
	std::vector<VkSemaphore> render_finished_semaphores;
	std::vector<VkFence> in_flight_fences;
	
	uint32_t current_frame = 0;

	void RecreateSwapchainHelper();

	const std::vector<Vertex> vertices_to_render = {
		{{0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}},
		{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
		{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
	};
};
} // namespace renderer