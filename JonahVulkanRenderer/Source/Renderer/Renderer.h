#pragma once

#include <memory>
#include <vector>
#include <array>
#include <string>

#include "Detail/RendererDetail.h"
#include "NewDetail/VkCommon.h"
#include "NewDetail/VkDrawSetup.h"
#include "NewDetail/VkDataSetup.h"

#ifdef NDEBUG
const bool UseValidationLayers = false;
#else
const bool UseValidationLayers = true;
#endif

namespace renderer {

class Renderer {
public:

	Renderer(int ScreenWidth, int ScreenHeight);
	~Renderer();

	void Draw(glm::mat4 CameraPosition, bool FrustumCull);
	void UpdateModelSet(std::vector<MeshInstances> NewModelSet, bool UseWhiteTexture);
	GLFWwindow* Get_Window();

	bool framebuffer_resized = false;

private:

	void RecordComputeCommands(uint32_t CurrentFrame, bool FrustumCull);
	void RecordGraphicsCommands(uint32_t CurrentFrame, uint32_t ImageIndex);
	void RecreateSwapchainHelper();

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

	uint32_t mesh_count;
	uint32_t unique_mesh_count;

	VkInstance vulkan_instance;
	VkSurfaceKHR vulkan_surface;
	VkPhysicalDevice physical_device;
	VkDevice logical_device;
	VkRenderPass render_pass;

	QueueFamilyIndices queues_supported;
	VkQueue graphics_queue;
	VkQueue compute_queue;
	VkQueue present_queue;

	VkDescriptorPool descriptor_pool;
	VkDescriptorSetLayout descriptor_layout;
	std::vector<VkDescriptorSet> descriptor_sets;

	draw::DepthBuffer depth_buffer;
	std::vector<VkFramebuffer> framebuffers;

	VkSurfaceFormatKHR swapchain_format;
	VkPresentModeKHR swapchain_present_mode;
	VkExtent2D swapchain_extent;
	uint32_t swapchain_image_count;

	VkSwapchainKHR swapchain;
	std::vector<VkImage> swapchain_images;
	std::vector<VkImageView> swapchain_image_views;

	data::Buffer vertex_buffer;
	data::Buffer index_buffer;
	data::Buffer instance_centers_buffer;
	data::Buffer instance_data_buffer;

	std::array<data::Buffer, MAX_FRAMES_IN_FLIGHT> should_draw_buffers;
	std::array<data::Buffer, MAX_FRAMES_IN_FLIGHT> indirect_command_buffers;
	std::array<data::UBO, MAX_FRAMES_IN_FLIGHT> uniform_buffers;

	std::vector<VkSemaphore> image_available_semaphores;
	std::vector<VkSemaphore> render_finished_semaphores;
	std::vector<VkFence> in_flight_fences;
	std::vector<VkFence> compute_in_flight_fences;
	std::vector<VkSemaphore> compute_finished_semaphores;

	VkPipelineLayout pipeline_layout;
	VkPipeline graphics_pipeline;
	VkPipeline compute_pipeline;
	VkCommandPool graphics_command_pool;
	VkCommandPool compute_command_pool;
	std::vector<VkCommandBuffer> graphics_command_buffers;
	std::vector<VkCommandBuffer> compute_command_buffers;

	PFN_vkCmdBeginDebugUtilsLabelEXT cmd_begin_debug = nullptr;
	PFN_vkCmdEndDebugUtilsLabelEXT cmd_end_debug = nullptr;
	
	uint32_t current_frame = 0;
};
} // namespace renderer