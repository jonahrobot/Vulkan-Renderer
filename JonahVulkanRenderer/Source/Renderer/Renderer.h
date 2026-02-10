#pragma once

#include <memory>
#include <vector>
#include <array>
#include <string>

#include "Detail/RendererDetail.h"
#include "NewDetail/VkCommon.h"
#include "NewDetail/VkDrawSetup.h"

#ifdef NDEBUG
const bool UseValidationLayers = false;
#else
const bool UseValidationLayers = true;
#endif

namespace renderer {

	#define WIDTH 800
	#define HEIGHT 600
	#define VERTEX_SHADER "shaders/vert.spv"
	#define FRAGMENT_SHADER "shaders/frag.spv"
	#define COMPUTE_SHADER "shaders/cull.spv"

	struct InstanceData {
		alignas(16) glm::mat4 model;
		alignas(16) glm::vec4 array_index;
	};

	struct UBOData {
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
		alignas(16) glm::vec4 frustum_planes[6];
	};

	struct Mesh {
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
	};

	struct MeshInstances {
		Mesh model_data;
		uint32_t instance_count = 0;
		std::vector<glm::mat4> instance_model_matrices;
	};

class Renderer {
public:
	Renderer();
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
	VkQueue graphics_queue;
	VkQueue compute_queue;
	VkQueue present_queue;
	VkRenderPass render_pass;

	draw::DepthBuffer depth_buffer;

	VkSurfaceFormatKHR swapchain_format;
	VkPresentModeKHR swapchain_present_mode;
	VkExtent2D swapchain_extent;
	uint32_t swapchain_image_count;
	VkSwapchainKHR swapchain;

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

	QueueFamilyIndices queues_supported;


	// BAD - Needs refactoring

	//detail::SwapchainContext swapchain_creation_data;

	std::vector<VkImage> swapchain_images;
	std::vector<VkImageView> swapchain_image_views;

	VkDescriptorPool descriptor_pool;
	std::vector<VkDescriptorSet> descriptor_sets;

	VkDescriptorSetLayout descriptor_layout;
	std::vector<VkFramebuffer> framebuffers;
	VkCommandPool graphics_command_pool;
	std::vector<VkCommandBuffer> graphics_command_buffers;

	VkCommandPool compute_command_pool;
	std::vector<VkCommandBuffer> compute_command_buffers;

	PFN_vkCmdBeginDebugUtilsLabelEXT cmd_begin_debug = nullptr;
	PFN_vkCmdEndDebugUtilsLabelEXT cmd_end_debug = nullptr;
	
	uint32_t current_frame = 0;
};
} // namespace renderer