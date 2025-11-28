#pragma once

#include <memory>
#include <algorithm>
#include <vector>
#include <iostream>
#include <array>
#include <string>

#include "../Renderer/Detail/RendererDetail.h"

#ifdef NDEBUG
const bool UseValidationLayers = false;
#else
const bool UseValidationLayers = true;
#endif

namespace renderer {

#define WIDTH 800
#define HEIGHT 600

class Renderer {
public:
	Renderer();
	~Renderer();

	void Draw(glm::mat4 CameraPosition);

	void UpdateModelSet(std::vector<detail::ModelData> NewModelSet);

	GLFWwindow* Get_Window();

	bool framebuffer_resized = false;

	struct UniformBufferObject {
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
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
	VkDescriptorPool descriptor_pool;
	std::vector<VkDescriptorSet> descriptor_sets;
	VkDescriptorSetLayout descriptor_set_layout;
	VkPipelineLayout graphics_pipeline_layout;
	std::vector<VkFramebuffer> framebuffers;
	VkCommandPool command_pool;
	std::vector<VkCommandBuffer> command_buffers;

	detail::GPUResource texture_0;

	VkSampler texture_sampler;

	VkBuffer vertex_buffer;
	VkDeviceMemory vertex_buffer_memory;
	VkBuffer index_buffer;
	VkDeviceMemory index_buffer_memory;
	VkBuffer indirect_command_buffer;
	VkDeviceMemory indirect_command_buffer_memory;

	std::vector<VkBuffer> uniform_buffers;
	std::vector<VkDeviceMemory> uniform_buffers_memory;
	std::vector<void*> uniform_buffers_mapped;
	
	detail::GPUResource depth_buffer;

	std::vector<VkSemaphore> image_available_semaphores;
	std::vector<VkSemaphore> render_finished_semaphores;
	std::vector<VkFence> in_flight_fences;
	
	uint32_t current_frame = 0;

	void RecreateSwapchainHelper();

	std::vector<detail::Vertex> vertices_to_render;
	std::vector<uint32_t>indices;

	const std::string MODEL_PATH = "models/viking_room.obj";
	const std::string TEXTURE_PATH = "textures/viking_room.png";
};
} // namespace renderer