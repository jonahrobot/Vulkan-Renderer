#pragma once

#include <memory>
#include <vector>
#include <array>
#include <string>

#include "Detail/RendererDetail.h"

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

	void Draw(glm::mat4 CameraPosition, bool FrustumCull);

	void UpdateModelSet(std::vector<detail::MeshInstances> NewModelSet, bool UseWhiteTexture);

	GLFWwindow* Get_Window();

	bool framebuffer_resized = false;

private:

	// GOOD - No refactor needed

	const std::vector<const char*> ValidationLayersToSupport = {
		"VK_LAYER_KHRONOS_validation"
	};

	std::vector<const char*> InstanceExtensionsToSupport = {
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME
	};

	const std::vector<const char*> DeviceExtensionsToSupport = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};

	GLFWwindow* window;

	VkDevice logical_device;
	detail::VulkanCore vulkan_core;
	detail::SwapchainCore swapchain_core;
	detail::SwapchainDependents swapchain_dependents;

	void FreeVulkanCore(detail::VulkanCore& VulkanCore);
	void FreeSwapchainCore(const detail::VulkanCore& VulkanCore, detail::SwapchainCore& SwapchainCore);

	uint32_t mesh_count;
	uint32_t unique_mesh_count;

	VkQueue graphics_queue;
	VkQueue compute_queue;
	VkQueue present_queue;

	detail::Buffer vertex_buffer;
	detail::Buffer index_buffer;
	detail::Buffer instance_centers_buffer;
	detail::Buffer instance_data_buffer;

	std::array<detail::Buffer, MAX_FRAMES_IN_FLIGHT> should_draw_buffers;
	std::array<detail::Buffer, MAX_FRAMES_IN_FLIGHT> indirect_command_buffers;
	std::array<detail::BufferMapped, MAX_FRAMES_IN_FLIGHT> uniform_buffers;

	std::vector<VkSemaphore> image_available_semaphores;
	std::vector<VkSemaphore> render_finished_semaphores;
	std::vector<VkFence> in_flight_fences;
	std::vector<VkFence> compute_in_flight_fences;
	std::vector<VkSemaphore> compute_finished_semaphores;

	detail::Pipeline graphics_pipeline;
	detail::Pipeline compute_pipeline;

	// BAD - Needs refactoring

	VkDescriptorPool descriptor_pool;
	std::vector<VkDescriptorSet> descriptor_sets;

	VkDescriptorSetLayout descriptor_set_layout;
	VkCommandPool command_pool;
	std::vector<VkCommandBuffer> command_buffers;

	VkCommandPool compute_command_pool;
	std::vector<VkCommandBuffer> compute_command_buffers;

	PFN_vkCmdBeginDebugUtilsLabelEXT pfn_CmdBeginDebugUtilsLabelEXT = nullptr;
	PFN_vkCmdEndDebugUtilsLabelEXT pfn_CmdEndDebugUtilsLabelEXT = nullptr;
	
	uint32_t current_frame = 0;

	void RecreateSwapchain();
};
} // namespace renderer