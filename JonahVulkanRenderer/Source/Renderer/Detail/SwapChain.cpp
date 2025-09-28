#include "RendererDetail.h"
#include <algorithm>
#include <iostream>

// Unnamed namespace to show functions below are pure Utility strictly for this .cpp file.
namespace {

	// Preferred: SRGB 8 bit
	VkSurfaceFormatKHR SelectSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {

		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	// Preferred: Mailbox present mode
	VkPresentModeKHR SelectPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	// Sets swapchain size to window size
	VkExtent2D SelectExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window) {
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

		const SwapChainSupportDetails* creation_options = &context.swapchain_support_details;

		uint32_t image_count = creation_options->capabilities.minImageCount + 1;
		uint32_t max_images = creation_options->capabilities.maxImageCount;
		bool has_max_image_count = max_images > 0;
		bool over_max_images = has_max_image_count && image_count > max_images;

		if (over_max_images) {
			image_count = max_images;
		}

		VkSurfaceFormatKHR surface_format = SelectSurfaceFormat(creation_options->formats);
		VkPresentModeKHR present_mode = SelectPresentMode(creation_options->presentModes);
		VkExtent2D extent = SelectExtent(creation_options->capabilities, context.window);

		VkSwapchainCreateInfoKHR create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		create_info.surface = context.vulkan_surface;
		create_info.minImageCount = image_count;
		create_info.imageFormat = surface_format.format;
		create_info.imageColorSpace = surface_format.colorSpace;
		create_info.imageExtent = extent;
		create_info.imageArrayLayers = 1; // Always 1 unless stereoscopic 3D.
		create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		uint32_t queue_family_indices[] = { context.supported_queues.graphicsFamily.value(), context.supported_queues.presentFamily.value() };

		// Must check if graphics and present queues are different.
		// If they are, we must handle interactions concurrently.
		if (context.supported_queues.graphicsFamily != context.supported_queues.presentFamily) {
			create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			create_info.queueFamilyIndexCount = 2;
			create_info.pQueueFamilyIndices = queue_family_indices;
		}
		else {
			create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		create_info.preTransform = context.swapchain_support_details.capabilities.currentTransform;
		create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		create_info.presentMode = present_mode;
		create_info.clipped = VK_TRUE; 
		create_info.oldSwapchain = VK_NULL_HANDLE;

		VkSwapchainKHR swap_chain;

		if (vkCreateSwapchainKHR(context.logical_device, &create_info, nullptr, &swap_chain) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create swap chain!");
		}

		return swap_chain;
	}

} // namespace renderer::detail