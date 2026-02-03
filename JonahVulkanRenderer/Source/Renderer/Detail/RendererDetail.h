#pragma once

#include <optional>
#include <vector>

#include "RendererDetail_Common.h"


// Internal Helper functions for renderer.cpp
// Public Rendering API available at: "Renderer.h"
namespace renderer::detail {

	// Detail namespace seperates implementation logic of utility functions into their own classes.
	// Renderer::detail naming convention tells us these are specific implementations for our renderer class.

	// All below functions construct parts of the Vulkan Renderer used in "Renderer.cpp"

#pragma region Shared Structs
	struct Pipeline {
		VkPipeline pipeline;
		VkPipelineLayout layout;
	};

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphics_compute_family;
		std::optional<uint32_t> present_family;
	};

	struct Buffer {
		VkBuffer buffer;
		VkDeviceMemory memory;
		uint64_t byte_size = 0;
	};

	struct BufferMapped {
		Buffer buffer;
		void* buffer_mapped;
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

#pragma endregion

#pragma region Vulkan Instance

	// Implemented in "Instance.cpp"
	struct Instance_Stage {
		VkInstance vulkan_instance;
		GLFWwindow* window;
	};
	Instance_Stage CreateVulkanInstance(GLFWwindow* window, bool UseValidationLayers, const std::vector<const char*>& ValidationLayersToSupport, const std::vector<const char*>& InstanceExtensions);

#pragma endregion

#pragma region Surface Creation

	struct Surface_Stage {
		VkInstance vulkan_instance;
		GLFWwindow* window;
		VkSurfaceKHR vulkan_surface;
	};
	Surface_Stage CreateVulkanSurface(const Instance_Stage& Context);

#pragma endregion

#pragma region Physical Device
	// Implemented in "PhysicalDevice.cpp"

	struct PhysicalDevice_Stage {
		VkInstance vulkan_instance;
		GLFWwindow* window;
		VkSurfaceKHR vulkan_surface;
		VkPhysicalDevice physical_device;
		QueueFamilyIndices queues_supported;
	};
	PhysicalDevice_Stage PickPhysicalDevice(const Surface_Stage& Context, const std::vector<const char*>& DeviceExtensionsToSupport);

	// NEED TO MOVE TO SWAPCHAIN CLASS!
	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};
	SwapChainSupportDetails GetDeviceSwapchainSupport(const VkPhysicalDevice physical_device, const VkSurfaceKHR current_surface);

#pragma endregion

#pragma region Logical Device
	// Implemented in "LogicalDevice.cpp"
	struct VulkanCore {
		VkInstance vulkan_instance;
		GLFWwindow* window;
		VkSurfaceKHR vulkan_surface;
		VkPhysicalDevice physical_device;
		QueueFamilyIndices queues_supported;
		VkDevice logical_device;
	};

	VulkanCore CreateLogicalDevice(const PhysicalDevice_Stage& Context, bool UseValidationLayers, const std::vector<const char*>& ValidationLayersToSupport, const std::vector<const char*>& DeviceExtensionsToSupport);
#pragma endregion

#pragma region Swapchain
	// Implemented in "Swapchain.cpp"

	struct SwapchainCore {
		VkSwapchainKHR swapchain;
		VkExtent2D swapchain_extent;
		VkFormat swapchain_image_format;
		std::vector<VkImage> swapchain_images;
		std::vector<VkImageView> swapchain_image_views;
	};
	SwapchainCore CreateSwapchain(const VulkanCore& VulkanCore);

	struct SwapchainDependents {
		VkRenderPass render_pass;
		DepthBuffer depth_buffer;
		std::vector<VkFramebuffer> framebuffers;
	};

	VkRenderPass CreateRenderPass(const VulkanCore& VulkanCore, const SwapchainCore& Swapchain);
	DepthBuffer CreateDepthBuffer(const VulkanCore& VulkanCore, const SwapchainCore& Swapchain);
	std::vector<VkFramebuffer> CreateFrameBuffers(const VulkanCore& VulkanCore, const SwapchainCore& Swapchain, const VkRenderPass& RenderPass, const DepthBuffer& DepthBuffer);

#pragma endregion

#pragma region Command Buffers
	// Implemented in "CommandBuffer.cpp"
	VkCommandPool CreateCommandPool(const VkDevice LogicalDevice, uint32_t GraphicsFamilyIndex);

	std::vector<VkCommandBuffer> CreateCommandBuffers(const int TotalFrames, const VkDevice LogicalDevice, const VkCommandPool CommandPool);

	struct CommandRecordingContext {
		uint64_t unique_mesh_count;
		std::vector<VkFramebuffer> framebuffers;
		Buffer indirect_command_buffer;
		Buffer vertex_buffer;
		Buffer index_buffer;
		VkRenderPass render_pass;
		Pipeline graphics_pipeline;
		VkCommandBuffer command_buffer;
		uint32_t image_write_index;
		VkExtent2D swapchain_extent;
		VkDescriptorSet current_descriptor_set;
		PFN_vkCmdBeginDebugUtilsLabelEXT debug_function_begin;
		PFN_vkCmdEndDebugUtilsLabelEXT debug_function_end;
	};
	void RecordCommandBuffer(const CommandRecordingContext& Context);

	struct Compute_CommandRecordingContext {
		VkCommandBuffer command_buffer;
		Pipeline compute_pipeline;
		VkDescriptorSet current_descriptor_set;
		uint32_t instance_count;
		PFN_vkCmdBeginDebugUtilsLabelEXT debug_function_begin;
		PFN_vkCmdEndDebugUtilsLabelEXT debug_function_end;
	};
	void RecordCommandBuffer(const Compute_CommandRecordingContext& Context);

#pragma endregion

#pragma region Frame Buffers
	// Implemented in "FrameBuffer.cpp"
	//struct FrameBufferContext {
	//	std::vector<VkImageView> image_views;
	//	VkRenderPass render_pass;
	//	VkExtent2D swapchain_extent;
	//	VkDevice logical_device;
	//	VkImageView depth_image_view;
	//};
	//std::vector<VkFramebuffer> CreateFramebuffers(const FrameBufferContext& Context);
#pragma endregion

#pragma region Synchronization Primitives
	// Implemented in "SyncPrimatives.cpp"
	VkSemaphore CreateVulkanSemaphore(VkDevice LogicalDevice);
	VkFence CreateVulkanFence(VkDevice LogicalDevice);
#pragma endregion

#pragma region Graphics and Compute Pipeline
	// Implemented in "Pipelines.cpp"

	struct GraphicsPipelineContext {
		VkRenderPass render_pass;
		VkDevice logical_device;
		VkExtent2D swapchain_extent;
		VkDescriptorSetLayout descriptor_set_layout;
	};
	Pipeline CreateGraphicsPipeline(const GraphicsPipelineContext& Context);

	struct ComputePipelineContext {
		VkDevice logical_device;
		VkDescriptorSetLayout descriptor_set_layout;
	};
	Pipeline CreateComputePipeline(const ComputePipelineContext& Context);

	void DestroyPipeline(const VkDevice& LogicalDevice, Pipeline& Target);

#pragma endregion

#pragma region Data Buffers
	// Implemented in "DataBuffer.cpp"

	struct BufferContext {
		VkDevice logical_device;
		VkPhysicalDevice physical_device;
		VkQueue graphics_queue;
		VkCommandPool command_pool;
	};

	template<typename T>
	Buffer CreateLocalBuffer(const BufferContext& Context, const std::vector<T>& Data, VkBufferUsageFlags UsageFlags);

	void DestroyBuffer(VkDevice LogicalDevice, Buffer& Buffer);
	void DestroyBuffer(VkDevice LogicalDevice, BufferMapped& Buffer);

	struct MappedBufferContext {
		uint16_t buffer_size;
		VkDevice logical_device;
		VkPhysicalDevice physical_device;
	};

	BufferMapped CreateMappedBuffer(const MappedBufferContext& Context);

	VkFormat FindDepthFormat(VkPhysicalDevice PhysicalDevice);

	//struct DepthBufferContext {
	//	VkDevice logical_device;
	//	VkPhysicalDevice physical_device;
	//	VkExtent2D swapchain_extent;
	//};
	//DepthBuffer CreateDepthBuffer(const DepthBufferContext& Context);


#pragma endregion

#pragma region Descriptor Sets
	// Implemented in "DescriptorSet.cpp"

	VkDescriptorPool CreateDescriptorPool(const VkDevice& LogicalDevice, uint8_t MaxFramesInFlight);
	VkDescriptorSetLayout CreateDescriptorLayout(const VkDevice& LogicalDevice);
	
	struct DescriptorCreateContext {
		VkDevice logical_device;
		VkDescriptorSetLayout descriptor_set_layout;
		VkDescriptorPool descriptor_pool;
	};
	std::vector<VkDescriptorSet> CreateDescriptorSets(const DescriptorCreateContext& Context);

	struct Graphic_DescriptorContext {
		VkDevice logical_device;
		Buffer instance_buffer;
		std::array<BufferMapped, MAX_FRAMES_IN_FLIGHT> uniform_buffers;
		std::array<Buffer, MAX_FRAMES_IN_FLIGHT> should_draw_flags_buffer;
	};
	std::vector<VkDescriptorSet> UpdateDescriptorSets(const Graphic_DescriptorContext& Context, std::vector<VkDescriptorSet> old_set);

	struct Compute_DescriptorContext {
		VkDevice logical_device;
		std::array<Buffer, MAX_FRAMES_IN_FLIGHT> indirect_draw_buffers;
		Buffer instance_centers;
	};
	std::vector<VkDescriptorSet> UpdateComputeUniqueDescriptor(const Compute_DescriptorContext& Context, std::vector<VkDescriptorSet> old_set);

#pragma endregion

};