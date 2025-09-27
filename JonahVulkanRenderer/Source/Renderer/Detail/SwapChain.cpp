#include "RendererDetail.h"
#include <algorithm>
#include <iostream>

// Unnamed namespace to show functions below are pure Utility strictly for this .cpp file.
namespace {

	// Preferred: SRGB 8 bit
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {

		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	// Preferred: Mailbox present mode
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	// Sets swapchain size to window size
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window) {
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			// Usual case: Swapchain size == window resolution
			return capabilities.currentExtent;
		}
		else {
			// Window manager wants us to pick scale of swapchain.
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}
}

// Implements all Vulkan SwapChain Creation functions in "RendererDetail.h" to be used in "Renderer.cpp"
namespace renderer::detail {

	VkSwapchainKHR CreateSwapChain(const SwapChainContext& context) {

		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(context.swapchain_support_details.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(context.swapchain_support_details.presentModes);
		VkExtent2D extent = chooseSwapExtent(context.swapchain_support_details.capabilities, context.window);
		uint32_t imageCount = context.swapchain_support_details.capabilities.minImageCount + 1;

		// maxImageCount of 0 means no max image count.
		if (context.swapchain_support_details.capabilities.maxImageCount > 0 && imageCount > context.swapchain_support_details.capabilities.maxImageCount) {
			imageCount = context.swapchain_support_details.capabilities.maxImageCount;
		}

		// Define Swap Chain Creation Struct
		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = context.vulkan_surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1; // Always 1 unless stereoscopic 3D.
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		uint32_t queueFamilyIndices[] = { context.supported_queues.graphicsFamily.value(), context.supported_queues.presentFamily.value() };

		if (context.supported_queues.graphicsFamily != context.supported_queues.presentFamily) {
			// Concurrent handling of queues when graphics and present logic
			// on different queues.
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			// Most common mode, graphic and present on same queue family.
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		// Modify if we want to apply transformations to images on swapchain
		createInfo.preTransform = context.swapchain_support_details.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE; // Don't render pixels obsured by another window.
		createInfo.oldSwapchain = VK_NULL_HANDLE; // Must use when swapchain becomes invalid (window resize)

		VkSwapchainKHR swapChain;

		if (vkCreateSwapchainKHR(context.logical_device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create swap chain!");
		}

		return swapChain;
	}

} // namespace renderer::detail