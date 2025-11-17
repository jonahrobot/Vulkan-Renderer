#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stb_master/stb_image.h>
#include <glm/glm.hpp>
#include <optional>
#include <vector>
#include <array>

#include "RendererDetail_Common.h"


// Internal Helper functions for renderer.cpp
// Public Rendering API available at: "Renderer.h"
namespace renderer::detail {

	// Detail namespace seperates implementation logic of utility functions into their own classes.
	// Renderer::detail naming convention tells us these are specific implementations for our renderer class.

#pragma region Vertex Type
	struct Vertex {
		glm::vec3 position;
		glm::vec3 color;
		glm::vec2 tex_coord;

		static VkVertexInputBindingDescription GetBindingDescription() {
			VkVertexInputBindingDescription binding_description{};
			binding_description.binding = 0;
			binding_description.stride = sizeof(Vertex);
			binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			return binding_description;
		}

		static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescription() {
			std::array<VkVertexInputAttributeDescription, 3> attribute_descriptions = {};

			// Position
			attribute_descriptions[0].binding = 0;
			attribute_descriptions[0].location = 0;
			attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT; // Vec3
			attribute_descriptions[0].offset = offsetof(Vertex, position);

			// Color
			attribute_descriptions[1].binding = 0;
			attribute_descriptions[1].location = 1;
			attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT; // Vec3
			attribute_descriptions[1].offset = offsetof(Vertex, color);

			// Texture Coordinates
			attribute_descriptions[2].binding = 0;
			attribute_descriptions[2].location = 2;
			attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT; // Vec2
			attribute_descriptions[2].offset = offsetof(Vertex, tex_coord);

			return attribute_descriptions;
		}
	};
#pragma endregion

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
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value();
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
		std::vector<VkImageView> OLD_swapchain_image_views;
		std::vector<VkFramebuffer> OLD_framebuffers;
	};
	struct RecreateSwapchainData {
		SwapchainData swapchain_data;
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
	};
	std::vector<VkFramebuffer> CreateFramebuffers(const FrameBufferContext& Context);
#pragma endregion

#pragma region Synchronization Primitives
	// Implemented in "SyncPrimatives.cpp"
	VkSemaphore CreateVulkanSemaphore(VkDevice LogicalDevice);
	VkFence CreateVulkanFence(VkDevice LogicalDevice);
#pragma endregion

#pragma region Graphics Pipeline
	// Implemented in "GraphicsPipeline.cpp"
	struct GraphicsPipelineContext {
		VkRenderPass render_pass;
		VkDevice logical_device;
		VkExtent2D swapchain_extent;
		VkDescriptorSetLayout vertex_descriptor_set_layout;
	};
	struct GraphicsPipelineData {
		VkPipeline pipeline;
		VkPipelineLayout layout;
	};
	GraphicsPipelineData CreateGraphicsPipeline(const GraphicsPipelineContext& Context);
#pragma endregion

#pragma region Data Buffers
	// Implemented in "DataBuffer.cpp"

	struct VertexBufferContext {
		std::vector<Vertex> vertices_to_render;
		VkDevice logical_device;
		VkPhysicalDevice physical_device;
		VkQueue graphics_queue;
		VkCommandPool command_pool;
	};
	BufferData CreateVertexBuffer(const VertexBufferContext& Context);

	struct IndexBufferContext {
		std::vector<uint16_t> indices;
		VkDevice logical_device;
		VkPhysicalDevice physical_device;
		VkQueue graphics_queue;
		VkCommandPool command_pool;
	};
	BufferData CreateIndexBuffer(const IndexBufferContext& Context);

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

	struct DepthBufferContext {
		VkDevice logical_device;
		VkPhysicalDevice physical_device;
		VkExtent2D swapchain_extent;
	};
	GPUResource CreateDepthBuffer(const DepthBufferContext& Context);

#pragma endregion

#pragma region Descriptor Sets
	// Implemented in "DescriptorSet.cpp"

	struct DescriptorLayoutContext {
		VkDevice logical_device;
	};
	VkDescriptorSetLayout CreateDescriptorLayout(const DescriptorLayoutContext& Context);

	struct DescriptorPoolContext {
		uint8_t max_frames_in_flight;
		VkDevice logical_device;
	};
	VkDescriptorPool CreateDescriptorPool(const DescriptorPoolContext& Context);

	struct DescriptorSetContext {
		std::vector<VkBuffer> uniform_buffers;
		uint16_t ubo_size;
		VkDescriptorSetLayout descriptor_set_layout;
		VkDescriptorPool descriptor_pool;
		uint8_t max_frames_in_flight;
		VkDevice logical_device;
		VkImageView texture_image_view;
		VkSampler texture_sampler;
	};
	std::vector<VkDescriptorSet> CreateDescriptorSets(const DescriptorSetContext& Context);

#pragma endregion

#pragma region Image Loading
	// Implemented in "ImageLoader.cpp"

	struct TextureBundle {
		stbi_uc* pixels;
		VkDeviceSize image_size;
		int width;
		int height;
		VkFormat format;
	};
	TextureBundle LoadTextureImage(const char* TexturePath);
	void FreeTextureBundle(TextureBundle& TextureBundle);

	struct ImageObjectContext {
		TextureBundle texture_bundle;
		VkDevice logical_device;
		VkPhysicalDevice physical_device;
		VkImageTiling data_tiling_mode;
		VkImageUsageFlags usage_flags;
		VkMemoryPropertyFlagBits memory_flags_required;
		VkQueue graphics_queue;
		VkCommandPool command_pool;
	};
	GPUResource CreateImageObject(const ImageObjectContext& Context);
	void FreeImageObject(GPUResource& ImageObject, const VkDevice& LogicalDevice);

#pragma endregion
};