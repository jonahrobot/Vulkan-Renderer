#pragma once

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include "VkCommon.h"

namespace renderer::swapchain {

	struct SwapchainOptions {
		VkSurfaceCapabilitiesKHR Capabilities;
		std::vector<VkSurfaceFormatKHR> Formats;
		std::vector<VkPresentModeKHR> PresentModes;
	};
	SwapchainOptions QuerySwapchainSupport(VkPhysicalDevice PhysicalDevice, VkSurfaceKHR CurrentSurface);

	VkSurfaceFormatKHR ChooseFormat(const SwapchainOptions& SwapchainOptions);
	VkPresentModeKHR ChoosePresentMode(const SwapchainOptions& SwapchainOptions);
	VkExtent2D ChooseExtent(const SwapchainOptions& SwapchainOptions, const GLFWwindow* Window);

	VkSwapchainKHR CreateSwapchain(VkDevice LogicalDevice, VkSurfaceKHR VulkanSurface, const QueueFamilyIndices& SupportedQueues);

	std::vector<VkImage> CreateSwapchainImages(VkDevice LogicalDevice, VkSwapchainKHR Swapchain);
	std::vector<VkImageView> CreateSwapchainViews(VkDevice LogicalDevice, VkFormat ImageFormat, const std::vector<VkImage>& Images);
}