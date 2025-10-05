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

	VkSwapchainKHR CreateSwapchain(OutParams_Swapchain& out_params, const SwapchainContext& context) {

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

		out_params.swapchain_image_format = surface_format.format;
		out_params.swapchain_extent = extent;

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

	void GetSwapchainImages(std::vector<VkImage>& out_Images, const VkSwapchainKHR Swapchain, const VkDevice LogicalDevice) {

		uint32_t image_count = 0;
		vkGetSwapchainImagesKHR(LogicalDevice, Swapchain, &image_count, nullptr);
		out_Images.resize(image_count);
		vkGetSwapchainImagesKHR(LogicalDevice, Swapchain, &image_count, out_Images.data());
		
		return;
	}

	void CreateSwapchainViews(std::vector<VkImageView>& out_ImageViews, const std::vector<VkImage>& Images, const VkDevice LogicalDevice, const VkFormat ImageFormat, const VkExtent2D Extent){

		out_ImageViews.resize(Images.size());

		for (size_t i = 0; i < Images.size(); i++) {

			VkImageViewCreateInfo create_info{};
			create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			create_info.image = Images[i];
			create_info.format = ImageFormat;
			
			// Set all to identity to avoid color alteration
			create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			create_info.subresourceRange.baseMipLevel = 0;
			create_info.subresourceRange.levelCount = 1;
			create_info.subresourceRange.baseArrayLayer = 0;
			create_info.subresourceRange.layerCount = 1;

			VkResult created = vkCreateImageView(LogicalDevice, &create_info, nullptr, &out_ImageViews[i]);
			if (created == false) {
				throw std::runtime_error("Failed to create Image View.");
			}
		}

		return;
	}

} // namespace renderer::detail