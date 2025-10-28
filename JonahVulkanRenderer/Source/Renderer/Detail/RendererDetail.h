#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <optional>
#include <vector>

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
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};
	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};
	struct PhysicalDeviceData {
		VkPhysicalDevice physical_device;
		QueueFamilyIndices queues_supported;
		SwapChainSupportDetails swapchain_support_details;
	};
	PhysicalDeviceData PickPhysicalDevice(const PhysicalDeviceContext& Context);
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
		SwapChainSupportDetails swapchain_support_details;
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
		VkCommandBuffer command_buffer;
		uint32_t image_write_index;
		VkExtent2D swapchain_extent;
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
	};
	struct GraphicsPipelineData {
		VkPipeline pipeline;
		VkPipelineLayout layout;
	};
	GraphicsPipelineData CreateGraphicsPipeline(const GraphicsPipelineContext& Context);
#pragma endregion

};