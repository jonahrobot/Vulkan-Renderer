#pragma once

#include <vulkan/vulkan.hpp>

namespace renderer::pipeline {

	VkRenderPass CreateRenderPass(VkDevice LogicalDevice, VkPhysicalDevice PhysicalDevice, VkFormat SwapchainFormat, VkSurfaceFormatKHR DepthBufferFormat);

	VkDescriptorSetLayout CreateDescriptorLayout(VkDevice LogicalDevice);
	VkDescriptorPool CreateDescriptorPool(VkDevice LogicalDevice);
	std::vector<VkDescriptorSet> CreateDescriptorSets(VkDevice LogicalDevice, VkDescriptorSetLayout Layout, VkDescriptorPool Pool);

	VkPipelineLayout CreatePipelineLayout(VkDevice LogicalDevice, VkDescriptorSetLayout DescriptorLayout);
	VkPipeline CreateGraphicsPipeline(VkDevice LogicalDevice, VkPipelineLayout Layout, VkRenderPass RenderPass, const char* VertexShaderPath, const char* FragmentShaderPath);
	VkPipeline CreateComputePipeline(VkDevice LogicalDevice, VkPipelineLayout Layout, const char* ComputeShaderPath);

}