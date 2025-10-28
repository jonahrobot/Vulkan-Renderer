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
	
	// Implemented in "Instance.cpp"
	VkInstance CreateVulkanInstance(const bool UseValidationLayers, const std::vector<const char*>& ValidationLayersToSupport);

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

	// Implemented in "SwapChain.cpp"
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

	// Implemented in "SwapChain.cpp"
	std::vector<VkImage> GetSwapchainImages(const VkSwapchainKHR Swapchain, const VkDevice LogicalDevice);

	// Implemented in "SwapChain.cpp"
	std::vector<VkImageView> CreateSwapchainViews(const std::vector<VkImage>& Images, const VkDevice LogicalDevice, const VkFormat& ImageFormat);

	// Implemented in "CommandBuffers.cpp"
	VkCommandPool CreateCommandPool(const VkDevice LogicalDevice, uint32_t GraphicsFamilyIndex);

	// Implemented in "CommandBuffers.cpp"
	std::vector<VkCommandBuffer> CreateCommandBuffers(const int TotalFrames, const VkDevice LogicalDevice, const VkCommandPool CommandPool);

	// Implemented in "CommandBuffers.cpp"
	struct CommandRecordingContext {
		std::vector<VkFramebuffer> framebuffers;
		VkRenderPass render_pass;
		VkPipeline graphics_pipeline;
		VkCommandBuffer command_buffer;
		uint32_t image_write_index;
		VkExtent2D swapchain_extent;
	};
	void RecordCommandBuffer(const CommandRecordingContext& Context);

	// Implemented in "SyncPrimatives.cpp"
	VkSemaphore CreateVulkanSemaphore(VkDevice LogicalDevice);

	// Implemented in "SyncPrimatives.cpp"
	VkFence CreateVulkanFence(VkDevice LogicalDevice);

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
};