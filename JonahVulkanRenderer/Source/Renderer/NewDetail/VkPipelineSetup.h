#pragma once

#include <vulkan/vulkan.hpp>

namespace renderer::pipeline {

	VkRenderPass CreateRenderPass(VkDevice LogicalDevice, VkPhysicalDevice PhysicalDevice, VkFormat SwapchainFormat);

	VkCommandPool CreateCommandPool(VkDevice LogicalDevice, uint32_t GraphicsFamilyIndex);
	std::vector<VkCommandBuffer> CreateCommandBuffers(VkDevice LogicalDevice, VkCommandPool CommandPool, int TotalFrames);

	VkDescriptorSetLayout CreateDescriptorLayout(VkDevice LogicalDevice);
	VkDescriptorPool CreateDescriptorPool(VkDevice LogicalDevice);
	std::vector<VkDescriptorSet> CreateDescriptorSets(VkDevice LogicalDevice, VkDescriptorSetLayout Layout, VkDescriptorPool Pool);

	VkPipelineLayout CreatePipelineLayout(VkDevice LogicalDevice, VkDescriptorSetLayout DescriptorLayout);
	VkPipeline CreateGraphicsPipeline(VkDevice LogicalDevice, VkPipelineLayout Layout);
	VkPipeline CreateComputePipeline(VkDevice LogicalDevice, VkPipelineLayout Layout, VkRenderPass RenderPass);

	VkSemaphore CreateVulkanSemaphore(VkDevice LogicalDevice);
	VkFence CreateVulkanFence(VkDevice LogicalDevice);

	struct DepthBuffer {
		VkImage Image;
		VkDeviceMemory ImageDeviceMemory;
		VkImageView ImageView;
	};
	DepthBuffer CreateDepthBuffer(VkDevice LogicalDevice, VkPhysicalDevice PhysicalDevice, VkExtent2D SwapchainExtent);
	std::vector<VkFramebuffer> CreateFramebuffers(VkDevice LogicalDevice, DepthBuffer DepthBuffer, VkRenderPass RenderPass, VkExtent2D SwapchainExtent, const std::vector<VkImageView>& SwapchainImageViews);
}