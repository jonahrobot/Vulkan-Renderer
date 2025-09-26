#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

// Helper functions for renderer.cpp
namespace renderer::detail {

	// Detail namespace seperates implementation logic of utility functions into their own classes.
	// Renderer::detail naming convention tells us these are specific implementations for our renderer class.
	
	// Implemented in "Instance.cpp"
	VkInstance CreateVulkanInstance(bool UseValidationLayers, const std::vector<const char*>& ValidationLayersToSupport);

	// Implemented in "PhysicalDevice.cpp"
	struct PhysicalDeviceContext {
		VkInstance vulkan_instance;
		VkSurfaceKHR vulkan_surface;
	};
	VkPhysicalDevice PickPhysicalDevice(const PhysicalDeviceContext& context, const std::vector<const char*>& DeviceExtensionToSupport);

	// Implemented in "LogicalDevice.cpp"
	struct LogicalDeviceContext {
		VkInstance vulkan_instance;
		VkSurfaceKHR vulkan_surface;
		VkPhysicalDevice physical_device;
	};
	VkDevice CreateLogicalDevice(const LogicalDeviceContext& context);

	// Implemented in "SwapChain.cpp"
	struct SwapChainContext {
		VkSurfaceKHR vulkan_surface;
		VkPhysicalDevice physical_device;
		VkDevice logical_device;
		GLFWwindow* window;
	};
	VkSwapchainKHR CreateSwapChain(const SwapChainContext& context);
};