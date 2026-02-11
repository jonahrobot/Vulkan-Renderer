#include <algorithm>

#include "VkSwapchainSetup.h"

namespace renderer::swapchain {
	SwapchainOptions QuerySwapchainSupport(VkPhysicalDevice PhysicalDevice, VkSurfaceKHR CurrentSurface) {

		SwapchainOptions swapchain_options;

		// 1. Get Swapchain Capabilities
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysicalDevice, CurrentSurface, &swapchain_options.Capabilities);

		// 2. Get Swapchain formats
		uint32_t supported_format_count;
		vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, CurrentSurface, &supported_format_count, nullptr);
		if (supported_format_count != 0) {
			swapchain_options.Formats.resize(supported_format_count);
			vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, CurrentSurface, &supported_format_count, swapchain_options.Formats.data());
		}

		// 3. Get Swapchain present modes
		uint32_t supported_present_mode_count;
		vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, CurrentSurface, &supported_present_mode_count, nullptr);
		if (supported_present_mode_count != 0) {
			swapchain_options.PresentModes.resize(supported_present_mode_count);
			vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, CurrentSurface, &supported_present_mode_count, swapchain_options.PresentModes.data());
		}

		return swapchain_options;
	}

	VkSurfaceFormatKHR ChooseFormat(const SwapchainOptions& SwapchainOptions) {

		for (const auto& format : SwapchainOptions.Formats) {
			if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return format;
			}
		}
		return SwapchainOptions.Formats[0];
	}

	VkPresentModeKHR ChoosePresentMode(const SwapchainOptions& SwapchainOptions) {

		for (const auto& present_mode : SwapchainOptions.PresentModes) {
			if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return present_mode;
			}
		}
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D ChooseExtent(const SwapchainOptions& SwapchainOptions, GLFWwindow* Window) {

		// In Vulkan if CurrentExtent = uint32 max, that flags a update is needed.
		bool swapchain_needs_update = SwapchainOptions.Capabilities.currentExtent.width == std::numeric_limits<uint32_t>::max();

		if (swapchain_needs_update == false) {
			return SwapchainOptions.Capabilities.currentExtent;
		}

		int width, height;
		glfwGetFramebufferSize(Window, &width, &height);

		VkExtent2D extent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		extent.width = std::clamp(extent.width, SwapchainOptions.Capabilities.minImageExtent.width, SwapchainOptions.Capabilities.maxImageExtent.width);
		extent.height = std::clamp(extent.height, SwapchainOptions.Capabilities.minImageExtent.height, SwapchainOptions.Capabilities.maxImageExtent.height);

		return extent;
	}

	uint32_t ChooseImageCount(const SwapchainOptions& SwapchainOptions) {

		uint32_t image_count = SwapchainOptions.Capabilities.minImageCount + 1;
		uint32_t max_images = SwapchainOptions.Capabilities.maxImageCount;

		if (max_images > 0) {
			image_count = std::clamp(image_count, 0u, max_images);
		}
		return image_count;
	}

	VkSwapchainKHR CreateSwapchain(
		VkDevice LogicalDevice,
		VkSurfaceKHR VulkanSurface,
		VkSurfaceFormatKHR Format,
		VkPresentModeKHR PresentMode,
		VkExtent2D Extent,
		uint32_t ImageCount,
		const SwapchainOptions& SwapchainOptions,
		const QueueFamilyIndices& SupportedQueues
	){

		VkSwapchainCreateInfoKHR create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		create_info.surface = VulkanSurface;
		create_info.minImageCount = ImageCount;
		create_info.imageFormat = Format.format;
		create_info.imageColorSpace = Format.colorSpace;
		create_info.imageExtent = Extent;
		create_info.imageArrayLayers = 1;
		create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		create_info.preTransform = SwapchainOptions.Capabilities.currentTransform;
		create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		create_info.presentMode = PresentMode;
		create_info.clipped = VK_TRUE;
		create_info.oldSwapchain = VK_NULL_HANDLE;

		uint32_t queue_family_indices[] = { SupportedQueues.graphics_compute_family.value(),  SupportedQueues.present_family.value() };

		if (SupportedQueues.graphics_compute_family != SupportedQueues.present_family) {
			create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			create_info.queueFamilyIndexCount = 2;
			create_info.pQueueFamilyIndices = queue_family_indices;
		}

		VkSwapchainKHR swapchain;
		if (vkCreateSwapchainKHR(LogicalDevice, &create_info, nullptr, &swapchain) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create swap chain.");
		}

		return swapchain;
	}

	std::vector<VkImage> CreateSwapchainImages(VkDevice LogicalDevice, VkSwapchainKHR Swapchain) {

		std::vector<VkImage> swapchain_images{};

		uint32_t image_count = 0;
		vkGetSwapchainImagesKHR(LogicalDevice, Swapchain, &image_count, nullptr);
		swapchain_images.resize(image_count);
		vkGetSwapchainImagesKHR(LogicalDevice, Swapchain, &image_count, swapchain_images.data());

		return swapchain_images;
	}

	std::vector<VkImageView> CreateSwapchainViews(VkDevice LogicalDevice, VkFormat SwapchainFormat, const std::vector<VkImage>& Images) {

		std::vector<VkImageView> swapchain_image_views{};

		swapchain_image_views.resize(Images.size());

		for (size_t i = 0; i < Images.size(); i++) {

			VkImageViewCreateInfo create_info{};
			create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			create_info.image = Images[i];

			create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			create_info.format = SwapchainFormat;

			create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			create_info.subresourceRange.baseMipLevel = 0;
			create_info.subresourceRange.levelCount = 1;
			create_info.subresourceRange.baseArrayLayer = 0;
			create_info.subresourceRange.layerCount = 1;

			VkResult created = vkCreateImageView(LogicalDevice, &create_info, nullptr, &swapchain_image_views[i]);
			if (created != VK_SUCCESS) {
				throw std::runtime_error("Failed to create Image View.");
			}
		}

		return swapchain_image_views;
	}
}