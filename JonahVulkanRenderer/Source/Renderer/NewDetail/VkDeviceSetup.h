#pragma once

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#include <vector>

#include "VkCommon.h"

namespace renderer::device {

	GLFWwindow* CreateWindow(const char* Title, int Width, int Height);

	VkInstance CreateVulkanInstance(bool UseValidationLayers, const std::vector<const char*>& ValidationLayersToSupport, const std::vector<const char*>& InstanceExtensions);

	VkSurfaceKHR CreateVulkanSurface(VkInstance VulkanInstance, GLFWwindow* Window);

	QueueFamilyIndices FindSupportedQueues(VkPhysicalDevice PhysicalDevice, VkSurfaceKHR CurrentSurface);

	VkPhysicalDevice PickPhysicalDevice(VkInstance VulkanInstance, VkSurfaceKHR VulkanSurface, const std::vector<const char*>& DeviceExtensionsToSupport);

	struct LogicalDeviceContext {
		VkPhysicalDevice PhysicalDevice;
		QueueFamilyIndices SupportedQueues;
		bool UseValidationLayers;
		std::vector<const char*> DeviceExtensionsToSupport;
		std::vector<const char*> ValidationLayersToSupport;
	};
	VkDevice CreateLogicalDevice(const LogicalDeviceContext& Context);
}