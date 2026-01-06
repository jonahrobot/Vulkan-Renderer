#pragma once

#include <stb_master/stb_image.h>
#include <tiny_obj_loader/tiny_obj_loader.h>
#include <optional>
#include <vector>

#include "RendererDetail_Common.h"


// Internal Helper functions for renderer.cpp
// Public Rendering API available at: "Renderer.h"
namespace renderer::detail {

	// Detail namespace seperates implementation logic of utility functions into their own classes.
	// Renderer::detail naming convention tells us these are specific implementations for our renderer class.

	// All below functions construct parts of the Vulkan Renderer used in "Renderer.cpp"

#pragma region Vulkan Instance
	// Implemented in "Instance.cpp"
	VkInstance CreateVulkanInstance(const bool UseValidationLayers, const std::vector<const char*>& ValidationLayersToSupport);
#pragma endregion

#pragma region Physical Device
	// Implemented in "PhysicalDevice.cpp"
	struct PhysicalDeviceContext {
		VkInstance vulkan_instance;
		VkSurfaceKHR vulkan_surface;
		std::vector<const char*> DeviceExtensionsToSupport;
	};
	struct QueueFamilyIndices {
		std::optional<uint32_t> graphics_compute_family;
		std::optional<uint32_t> present_family;

		bool isComplete() {
			return graphics_compute_family.has_value() && present_family.has_value();
		}
	};

	struct PhysicalDeviceData {
		VkPhysicalDevice physical_device;
		QueueFamilyIndices queues_supported;
	};
	PhysicalDeviceData PickPhysicalDevice(const PhysicalDeviceContext& Context);

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};
	SwapChainSupportDetails GetDeviceSwapchainSupport(const VkPhysicalDevice physical_device, const VkSurfaceKHR current_surface);

#pragma endregion

#pragma region Logical Device
	// Implemented in "LogicalDevice.cpp"
	struct LogicalDeviceContext {
		VkInstance vulkan_instance;
		VkSurfaceKHR vulkan_surface;
		VkPhysicalDevice physical_device;
		QueueFamilyIndices supported_queues;
		bool UseValidationLayers;
		std::vector<const char*> DeviceExtensionsToSupport;
		std::vector<const char*> ValidationLayersToSupport;
	};
	VkDevice CreateLogicalDevice(const LogicalDeviceContext& Context);
#pragma endregion

#pragma region Swapchain
	// Implemented in "Swapchain.cpp"
	struct SwapchainContext {
		VkSurfaceKHR vulkan_surface;
		VkPhysicalDevice physical_device;
		VkDevice logical_device;
		GLFWwindow* window;
		QueueFamilyIndices supported_queues;
	};
	struct SwapchainData {
		VkSwapchainKHR swapchain;
		VkFormat swapchain_image_format;
		VkExtent2D swapchain_extent;
	};
	SwapchainData CreateSwapchain(const SwapchainContext& Context);

	std::vector<VkImage> GetSwapchainImages(const VkSwapchainKHR Swapchain, const VkDevice LogicalDevice);

	std::vector<VkImageView> CreateSwapchainViews(const std::vector<VkImage>& Images, const VkDevice LogicalDevice, const VkFormat& ImageFormat);

	struct RecreateSwapchainContext {
		SwapchainContext swapchain_creation_data;
		VkRenderPass render_pass;

		VkSwapchainKHR OLD_swapchain;
		GPUResource OLD_depth_buffer;
		std::vector<VkImageView> OLD_swapchain_image_views;
		std::vector<VkFramebuffer> OLD_framebuffers;
	};
	struct RecreateSwapchainData {
		SwapchainData swapchain_data;
		GPUResource depth_buffer;
		std::vector<VkImage> swapchain_images;
		std::vector<VkImageView> swapchain_image_views;
		std::vector<VkFramebuffer> framebuffers;
	};
	RecreateSwapchainData RecreateSwapchain(RecreateSwapchainContext Context);
#pragma endregion

#pragma region Command Buffers
	// Implemented in "CommandBuffer.cpp"
	VkCommandPool CreateCommandPool(const VkDevice LogicalDevice, uint32_t GraphicsFamilyIndex);

	std::vector<VkCommandBuffer> CreateCommandBuffers(const int TotalFrames, const VkDevice LogicalDevice, const VkCommandPool CommandPool);

	struct CommandRecordingContext {
		std::vector<VkFramebuffer> framebuffers;
		VkBuffer indirect_command_buffer;
		uint32_t number_of_draw_calls;
		VkRenderPass render_pass;
		VkPipeline graphics_pipeline;
		VkPipelineLayout graphics_pipeline_layout;
		VkCommandBuffer command_buffer;
		uint32_t image_write_index;
		VkExtent2D swapchain_extent;
		VkBuffer vertex_buffer;
		VkBuffer index_buffer;
		uint32_t total_indices;
		uint32_t total_vertices;
		VkDescriptorSet current_descriptor_set;
	};
	void RecordCommandBuffer(const CommandRecordingContext& Context);
#pragma endregion

#pragma region Frame Buffers
	// Implemented in "FrameBuffer.cpp"
	struct FrameBufferContext {
		std::vector<VkImageView> image_views;
		VkRenderPass render_pass;
		VkExtent2D swapchain_extent;
		VkDevice logical_device;
		VkImageView depth_image_view;
	};
	std::vector<VkFramebuffer> CreateFramebuffers(const FrameBufferContext& Context);
#pragma endregion

#pragma region Synchronization Primitives
	// Implemented in "SyncPrimatives.cpp"
	VkSemaphore CreateVulkanSemaphore(VkDevice LogicalDevice);
	VkFence CreateVulkanFence(VkDevice LogicalDevice);
#pragma endregion

#pragma region Graphics and Compute Pipeline
	// Implemented in "Pipelines.cpp"

	struct PipelineData {
		VkPipeline pipeline;
		VkPipelineLayout layout;
	};

	struct GraphicsPipelineContext {
		VkRenderPass render_pass;
		VkDevice logical_device;
		VkExtent2D swapchain_extent;
		VkDescriptorSetLayout descriptor_set_layout;
	};
	PipelineData CreateGraphicsPipeline(const GraphicsPipelineContext& Context);

	struct ComputePipelineContext {
		VkDevice logical_device;
		VkDescriptorSetLayout descriptor_set_layout;
	};
	PipelineData CreateComputePipeline(const ComputePipelineContext& Context);

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
	BufferData CreateLocalBuffer(const BufferContext& Context, const std::vector<T>& Data);

	struct UniformBufferContext {
		uint8_t max_frames_in_flight;
		uint16_t ubo_size;
		VkDevice logical_device;
		VkPhysicalDevice physical_device;
	};
	struct UniformBufferData {
		std::vector<VkBuffer> uniform_buffers;
		std::vector<VkDeviceMemory> uniform_buffers_memory;
		std::vector<void*> uniform_buffers_mapped;
	};
	UniformBufferData CreateUniformBuffers(const UniformBufferContext& Context);

	VkFormat FindDepthFormat(VkPhysicalDevice PhysicalDevice);

	struct DepthBufferContext {
		VkDevice logical_device;
		VkPhysicalDevice physical_device;
		VkExtent2D swapchain_extent;
	};
	GPUResource CreateDepthBuffer(const DepthBufferContext& Context);


#pragma endregion

#pragma region Descriptor Sets
	// Implemented in "DescriptorSet.cpp"

	VkDescriptorPool CreateDescriptorPool(const VkDevice& LogicalDevice, uint8_t MaxFramesInFlight);
	VkDescriptorSetLayout CreateDescriptorLayout(const VkDevice& LogicalDevice);
	
	struct DescriptorCreateContext {
		VkDevice logical_device;
		VkDescriptorSetLayout descriptor_set_layout;
		VkDescriptorPool descriptor_pool;
		uint8_t max_frames_in_flight;
	};
	std::vector<VkDescriptorSet> CreateDescriptorSets(const DescriptorCreateContext& Context);

	struct Graphic_DescriptorContext {
		VkDevice logical_device;
		uint8_t max_frames_in_flight;
		std::vector<VkBuffer> uniform_buffers;
		uint16_t ubo_size;
		VkImageView image_view;
		VkSampler texture_sampler;
		VkBuffer instance_buffer;
		uint64_t instance_buffer_size;
	};
	std::vector<VkDescriptorSet> UpdateDescriptorSets(const Graphic_DescriptorContext& Context, std::vector<VkDescriptorSet> old_set);

	struct Compute_DescriptorContext {
		VkDevice logical_device;
		uint8_t max_frames_in_flight;
		std::vector<VkBuffer> uniform_buffers;
		uint16_t ubo_size;
		VkBuffer instance_data_buffer;
		uint64_t instance_data_buffer_size;
		VkBuffer indirect_draw_buffer;
		uint64_t indirect_draw_buffer_size;
	};
	std::vector<VkDescriptorSet> UpdateDescriptorSets(const Compute_DescriptorContext& Context, std::vector<VkDescriptorSet> old_set);

#pragma endregion

#pragma region Asset Loading
	// Implemented in "AssetLoader.cpp"

	struct TextureData {
		stbi_uc* pixels;
		VkDeviceSize image_size;
		int width;
		int height;
		VkFormat format;
	};
	struct ModelData {
		std::vector<detail::Vertex> vertices;
		std::vector<uint32_t> indices;
		TextureData texture_data;
	};
	struct ModelWithUsage {
		std::string model_name;
		ModelData model_data;
		uint32_t instance_count;
		std::vector<glm::mat4> instance_model_matrices;
	};

	ModelData LoadModel(std::string ModelPath, const char* TexturePath);
	void FreeModel(ModelData Model);
	bool VerifyModel(ModelData Model);
	void PrintModelWithUsage(ModelWithUsage target);

	struct TextureBufferContext {
		TextureData texture_bundle;
		VkDevice logical_device;
		VkPhysicalDevice physical_device;
		VkImageTiling data_tiling_mode;
		VkImageUsageFlags usage_flags;
		VkMemoryPropertyFlagBits memory_flags_required;
		VkQueue graphics_queue;
		VkCommandPool command_pool;
		uint32_t number_of_textures;
	};
	GPUResource CreateTextureBuffer(const TextureBufferContext& Context);
	void FreeGPUResource(GPUResource& ImageObject, const VkDevice& LogicalDevice);

#pragma endregion

};