#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <optional>
#include <vector>

// Helper functions for renderer.cpp
namespace renderer::detail {

	// Detail namespace seperates implementation logic of utility functions into their own classes.
	// Renderer::detail naming convention tells us these are specific implementations for our renderer class.
	
	// Implemented in "Instance.cpp"
	VkInstance CreateVulkanInstance(const bool UseValidationLayers, const std::vector<const char*>& ValidationLayersToSupport);

	// Implemented in "PhysicalDevice.cpp"
	struct PhysicalDeviceContext {
		VkInstance vulkan_instance;
		VkSurfaceKHR vulkan_surface;
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
	struct DeviceSupportData {
		QueueFamilyIndices queues_supported;
		SwapChainSupportDetails swapchain_support_details;
	};
	VkPhysicalDevice PickPhysicalDevice(DeviceSupportData& out_DeviceSupportData, const PhysicalDeviceContext& Context, const std::vector<const char*>& DeviceExtensionsToSupport);

	// Implemented in "LogicalDevice.cpp"
	struct LogicalDeviceContext {
		VkInstance vulkan_instance;
		VkSurfaceKHR vulkan_surface;
		VkPhysicalDevice physical_device;
		QueueFamilyIndices supported_queues;
		bool UseValidationLayers;
	};
	VkDevice CreateLogicalDevice(const LogicalDeviceContext& Context, const std::vector<const char*>& DeviceExtensionsToSupport, const std::vector<const char*>& ValidationLayersToSupport);

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
		VkFormat swapchain_image_format;
		VkExtent2D swapchain_extent;
	};
	VkSwapchainKHR CreateSwapchain(SwapchainData& out_SwapchainData, const SwapchainContext& Context);

	// Implemented in "SwapChain.cpp"
	void GetSwapchainImages(std::vector<VkImage>& out_Images, const VkSwapchainKHR Swapchain, const VkDevice LogicalDevice);

	// Implemented in "SwapChain.cpp"
	void CreateSwapchainViews(std::vector<VkImageView>& out_ImageViews, const std::vector<VkImage>& Images, const VkDevice LogicalDevice, const VkFormat& ImageFormat);

	// Implemented in "GraphicsPipeline.cpp"
	VkPipeline CreateGraphicsPipeline(VkPipelineLayout& out_Layout, const VkRenderPass RenderPass, const VkDevice LogicalDevice, const VkExtent2D& SwapChainExtent);
};