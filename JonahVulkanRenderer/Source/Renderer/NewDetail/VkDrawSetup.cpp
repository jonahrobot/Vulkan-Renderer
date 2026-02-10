#include "VkDrawSetup.h"

namespace {

	// Used in CreateDepthBuffer()
	VkFormat FindDepthFormat(VkPhysicalDevice PhysicalDevice, VkImageTiling DesiredTiling, VkFormatFeatureFlags DesiredFeatures) {

		std::vector<VkFormat> possible_formats = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };

		for (VkFormat format : possible_formats) {
			VkFormatProperties properties;
			vkGetPhysicalDeviceFormatProperties(PhysicalDevice, format, &properties);

			bool support_for_linear_tiling = (properties.linearTilingFeatures & DesiredFeatures) == DesiredFeatures;
			bool support_for_optimal_tiling = (properties.optimalTilingFeatures & DesiredFeatures) == DesiredFeatures;

			if (DesiredTiling == VK_IMAGE_TILING_LINEAR && support_for_linear_tiling) {
				return format;
			}
			else if (DesiredTiling == VK_IMAGE_TILING_OPTIMAL && support_for_optimal_tiling) {
				return format;
			}
		}

		throw std::runtime_error("Failed to find a supported Depth format.");
	}

	// Used in CreateDepthBuffer()
	uint32_t FindMemoryType(VkPhysicalDevice PhysicalDevice, uint32_t TypeFilter, VkMemoryPropertyFlags Properties) {

		VkPhysicalDeviceMemoryProperties memory_properties;
		vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &memory_properties);

		for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {
			uint8_t check_one = TypeFilter & (1 << i);
			uint8_t check_two = memory_properties.memoryTypes[i].propertyFlags & Properties;
			if (check_one && check_two == Properties) {
				return i;
			}
		}

		throw std::runtime_error("Failed to find suitable memory type.");
	}
}

namespace renderer::draw {

	VkCommandPool CreateCommandPool(VkDevice LogicalDevice, uint32_t GraphicsFamilyIndex) {
		VkCommandPoolCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		create_info.queueFamilyIndex = GraphicsFamilyIndex;

		VkCommandPool graphics_command_pool;
		if (vkCreateCommandPool(LogicalDevice, &create_info, nullptr, &graphics_command_pool) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create command pool.");
		}
		return graphics_command_pool;
	}

	std::vector<VkCommandBuffer> CreateCommandBuffers(VkDevice LogicalDevice, VkCommandPool CommandPool, int TotalFrames) {
		std::vector<VkCommandBuffer> graphics_command_buffers;
		graphics_command_buffers.resize(TotalFrames);

		VkCommandBufferAllocateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		create_info.commandPool = CommandPool;
		create_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		create_info.commandBufferCount = (uint32_t)TotalFrames;

		if (vkAllocateCommandBuffers(LogicalDevice, &create_info, graphics_command_buffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate command buffers.");
		}
		return graphics_command_buffers;
	}

	VkSemaphore CreateVulkanSemaphore(VkDevice LogicalDevice) {
		VkSemaphoreCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkSemaphore semaphore;
		if (vkCreateSemaphore(LogicalDevice, &create_info, nullptr, &semaphore) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create semaphore.");
		}
		return semaphore;
	}

	VkFence CreateVulkanFence(VkDevice LogicalDevice) {
		VkFenceCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		VkFence fence;
		if (vkCreateFence(LogicalDevice, &create_info, nullptr, &fence) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create fence.");
		}
		return fence;
	}

	DepthBuffer CreateDepthBuffer(VkDevice LogicalDevice, VkPhysicalDevice PhysicalDevice, VkExtent2D SwapchainExtent) {

		// Function will define below three variables then return inside a DepthBuffer struct.
		VkImage depth_image = VK_NULL_HANDLE;
		VkDeviceMemory depth_image_memory = VK_NULL_HANDLE;
		VkImageView depth_image_view = VK_NULL_HANDLE;
		VkFormat depth_image_format;

		// 1. Create depth image
		depth_image_format = FindDepthFormat(PhysicalDevice, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

		VkImageCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		create_info.imageType = VK_IMAGE_TYPE_2D;
		create_info.extent.width = SwapchainExtent.width;
		create_info.extent.height = SwapchainExtent.height;
		create_info.extent.depth = 1;
		create_info.mipLevels = 1;
		create_info.arrayLayers = 1;
		create_info.format = depth_image_format;
		create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
		create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		create_info.samples = VK_SAMPLE_COUNT_1_BIT;
		create_info.flags = 0;

		if (vkCreateImage(LogicalDevice, &create_info, nullptr, &depth_image) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create image.");
		}
		
		// 2. Create depth image memory
		VkMemoryRequirements memory_requirements;
		vkGetImageMemoryRequirements(LogicalDevice, depth_image, &memory_requirements);

		VkMemoryAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		alloc_info.allocationSize = memory_requirements.size;
		alloc_info.memoryTypeIndex = FindMemoryType(PhysicalDevice, memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		if (vkAllocateMemory(LogicalDevice, &alloc_info, nullptr, &depth_image_memory) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate image memory.");
		}

		vkBindImageMemory(LogicalDevice, depth_image, depth_image_memory, 0);

		// 3. Create depth image view
		VkImageViewCreateInfo view_info{};
		view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view_info.image = depth_image;
		view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		view_info.format = depth_image_format;
		view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		view_info.subresourceRange.baseMipLevel = 0;
		view_info.subresourceRange.levelCount = 1;
		view_info.subresourceRange.baseArrayLayer = 0;
		view_info.subresourceRange.layerCount = 1;

		if (vkCreateImageView(LogicalDevice, &view_info, nullptr, &depth_image_view) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create image view.");
		}

		return { depth_image, depth_image_memory, depth_image_view, depth_image_format };
	}

	void DestroyDepthBuffer(VkDevice LogicalDevice, DepthBuffer& Instance) {
		vkDestroyImageView(LogicalDevice, Instance.ImageView, nullptr);
		vkDestroyImage(LogicalDevice, Instance.Image, nullptr);
		vkFreeMemory(LogicalDevice, Instance.ImageDeviceMemory, nullptr);
	}

	std::vector<VkFramebuffer> CreateFramebuffers(VkDevice LogicalDevice, DepthBuffer DepthBuffer, VkRenderPass RenderPass, VkExtent2D SwapchainExtent, const std::vector<VkImageView>& SwapchainImageViews) {
		
		std::vector<VkFramebuffer> frame_buffers(SwapchainImageViews.size());

		for (size_t i = 0; i < SwapchainImageViews.size(); i++) {

			std::array<VkImageView, 2> attachments = {
				SwapchainImageViews[i],
				DepthBuffer.ImageView
			};

			VkFramebufferCreateInfo create_info{};
			create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			create_info.renderPass = RenderPass;
			create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
			create_info.pAttachments = attachments.data();
			create_info.width = SwapchainExtent.width;
			create_info.height = SwapchainExtent.height;
			create_info.layers = 1;

			if (vkCreateFramebuffer(LogicalDevice, &create_info, nullptr, &frame_buffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create framebuffer.");
			}
		}

		return frame_buffers;
	}

}