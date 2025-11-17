#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <iostream>

namespace renderer::detail {

	struct GPUResource {
		VkImage texture_image;
		VkDeviceMemory texture_image_memory;
		VkImageView texture_image_view;
	};

	uint32_t FindMemoryType(VkPhysicalDevice PhysicalDevice, uint32_t TypeFilter, VkMemoryPropertyFlags properties);

	struct BufferData {
		VkBuffer created_buffer;
		VkDeviceMemory memory_allocated_for_buffer;
	};
	BufferData CreateDataBuffer(VkDevice LogicalDevice, VkPhysicalDevice PhysicalDevice, VkDeviceSize BufferSize, VkBufferUsageFlags UsageFlags, VkMemoryPropertyFlags PropertyFlags);

	VkCommandBuffer BeginSingleTimeCommand(VkCommandPool CommandPool, VkDevice LogicalDevice);

	void EndSingleTimeCommand(VkCommandBuffer CommandBuffer, VkCommandPool CommandPool, VkDevice LogicalDevice, VkQueue GraphicsQueue);
}