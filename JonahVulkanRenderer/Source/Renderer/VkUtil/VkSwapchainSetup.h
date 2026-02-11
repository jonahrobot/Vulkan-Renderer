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
	VkExtent2D ChooseExtent(const SwapchainOptions& SwapchainOptions, GLFWwindow* Window);
	uint32_t ChooseImageCount(const SwapchainOptions& SwapchainOptions);
	
	VkSwapchainKHR CreateSwapchain(
		VkDevice LogicalDevice, 
		VkSurfaceKHR VulkanSurface, 
		VkSurfaceFormatKHR Format, 
		VkPresentModeKHR PresentMode, 
		VkExtent2D Extent, 
		uint32_t ImageCount,
		const SwapchainOptions& SwapchainOptions,
		const QueueFamilyIndices& SupportedQueues
	);

	std::vector<VkImage> CreateSwapchainImages(VkDevice LogicalDevice, VkSwapchainKHR Swapchain);
	std::vector<VkImageView> CreateSwapchainViews(VkDevice LogicalDevice, VkFormat ImageFormat, const std::vector<VkImage>& Images);
}