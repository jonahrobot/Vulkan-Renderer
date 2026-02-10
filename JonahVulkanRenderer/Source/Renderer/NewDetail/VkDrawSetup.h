#pragma once

#include <vulkan/vulkan.hpp>

namespace renderer::draw {

	VkCommandPool CreateCommandPool(VkDevice LogicalDevice, uint32_t GraphicsFamilyIndex);
	std::vector<VkCommandBuffer> CreateCommandBuffers(VkDevice LogicalDevice, VkCommandPool CommandPool, int TotalFrames);

	VkSemaphore CreateVulkanSemaphore(VkDevice LogicalDevice);
	VkFence CreateVulkanFence(VkDevice LogicalDevice);

	struct DepthBuffer {
		VkImage Image;
		VkDeviceMemory ImageDeviceMemory;
		VkImageView ImageView;
		VkFormat ImageFormat;
	};
	DepthBuffer CreateDepthBuffer(VkDevice LogicalDevice, VkPhysicalDevice PhysicalDevice, VkExtent2D SwapchainExtent);
	void DestroyDepthBuffer(VkDevice LogicalDevice, DepthBuffer& Instance);

	std::vector<VkFramebuffer> CreateFramebuffers(VkDevice LogicalDevice, DepthBuffer DepthBuffer, VkRenderPass RenderPass, VkExtent2D SwapchainExtent, const std::vector<VkImageView>& SwapchainImageViews);

}