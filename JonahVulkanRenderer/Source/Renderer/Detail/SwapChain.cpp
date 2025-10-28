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

	SwapchainData CreateSwapchain(const SwapchainContext& Context) {

		const SwapChainSupportDetails* creation_options = &Context.swapchain_support_details;

		uint32_t image_count = creation_options->capabilities.minImageCount + 1;
		uint32_t max_images = creation_options->capabilities.maxImageCount;
		bool has_max_image_count = max_images > 0;
		bool over_max_images = has_max_image_count && image_count > max_images;

		if (over_max_images) {
			image_count = max_images;
		}

		VkSurfaceFormatKHR surface_format = SelectSurfaceFormat(creation_options->formats);
		VkPresentModeKHR present_mode = SelectPresentMode(creation_options->presentModes);
		VkExtent2D extent = SelectExtent(creation_options->capabilities, Context.window);

		VkSwapchainCreateInfoKHR create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		create_info.surface = Context.vulkan_surface;
		create_info.minImageCount = image_count;
		create_info.imageFormat = surface_format.format;
		create_info.imageColorSpace = surface_format.colorSpace;
		create_info.imageExtent = extent;
		create_info.imageArrayLayers = 1; // Always 1 unless stereoscopic 3D.
		create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		uint32_t queue_family_indices[] = { Context.supported_queues.graphicsFamily.value(), Context.supported_queues.presentFamily.value() };

		// Must check if graphics and present queues are different.
		// If they are, we must handle interactions concurrently.
		if (Context.supported_queues.graphicsFamily != Context.supported_queues.presentFamily) {
			create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			create_info.queueFamilyIndexCount = 2;
			create_info.pQueueFamilyIndices = queue_family_indices;
		}
		else {
			create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		create_info.preTransform = Context.swapchain_support_details.capabilities.currentTransform;
		create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		create_info.presentMode = present_mode;
		create_info.clipped = VK_TRUE; 
		create_info.oldSwapchain = VK_NULL_HANDLE;

		VkSwapchainKHR swapchain;

		if (vkCreateSwapchainKHR(Context.logical_device, &create_info, nullptr, &swapchain) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create swap chain!");
		}

		SwapchainData return_data{};

		return_data.swapchain_image_format = surface_format.format;
		return_data.swapchain_extent = extent;
		return_data.swapchain = swapchain;

		return return_data;
	}

	std::vector<VkImage> GetSwapchainImages(const VkSwapchainKHR Swapchain, const VkDevice LogicalDevice) {

		std::vector<VkImage> images{};

		uint32_t image_count = 0;
		vkGetSwapchainImagesKHR(LogicalDevice, Swapchain, &image_count, nullptr);
		images.resize(image_count);
		vkGetSwapchainImagesKHR(LogicalDevice, Swapchain, &image_count, images.data());
		
		return images;
	}

	std::vector<VkImageView> CreateSwapchainViews(const std::vector<VkImage>& Images, const VkDevice LogicalDevice, const VkFormat& ImageFormat){

		std::vector<VkImageView> image_views{};

		image_views.resize(Images.size());

		for (size_t i = 0; i < Images.size(); i++) {

			VkImageViewCreateInfo create_info{};
			create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			create_info.image = Images[i];

			create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
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

			VkResult created = vkCreateImageView(LogicalDevice, &create_info, nullptr, &image_views[i]);
			if (created != VK_SUCCESS) {
				throw std::runtime_error("Failed to create Image View.");
			}
		}

		return image_views;
	}

	RecreateSwapchainData RecreateSwapchain(RecreateSwapchainContext Context) {

		RecreateSwapchainData out_data = {};

		VkDevice logical_device = Context.swapchain_context.logical_device;

		vkDeviceWaitIdle(logical_device);

		// Cleanup old swapchain data

		for (auto framebuffer : Context.OLD_framebuffers) {
			vkDestroyFramebuffer(logical_device, framebuffer, nullptr);
		}
		for (auto view : Context.OLD_swapchain_image_views) {
			vkDestroyImageView(logical_device, view, nullptr);
		}
		vkDestroySwapchainKHR(logical_device, Context.OLD_swapchain, nullptr);

		// Create new swapchain data

		out_data.swapchain_data = detail::CreateSwapchain(Context.swapchain_context);

		out_data.swapchain_images = detail::GetSwapchainImages(out_data.swapchain_data.swapchain, logical_device);

		out_data.swapchain_image_views = detail::CreateSwapchainViews(out_data.swapchain_images, logical_device, out_data.swapchain_data.swapchain_image_format);

		detail::FrameBufferContext context_framebuffer = {};
		context_framebuffer.image_views = out_data.swapchain_image_views;
		context_framebuffer.render_pass = Context.render_pass;
		context_framebuffer.swapchain_extent = out_data.swapchain_data.swapchain_extent;
		context_framebuffer.logical_device = logical_device;

		out_data.framebuffers = detail::CreateFramebuffers(context_framebuffer);

		return out_data;
	}

} // namespace renderer::detail