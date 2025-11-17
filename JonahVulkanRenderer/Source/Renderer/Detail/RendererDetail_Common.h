#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <iostream>

namespace renderer::detail {

#pragma region Buffer Creation
	struct BufferData {
		VkBuffer created_buffer;
		VkDeviceMemory memory_allocated_for_buffer;
	};
	BufferData CreateDataBuffer(VkDevice LogicalDevice, VkPhysicalDevice PhysicalDevice, VkDeviceSize BufferSize, VkBufferUsageFlags UsageFlags, VkMemoryPropertyFlags PropertyFlags);

	uint32_t FindMemoryType(VkPhysicalDevice PhysicalDevice, uint32_t TypeFilter, VkMemoryPropertyFlags properties);

#pragma endregion

#pragma region Command Buffer Recording

	VkCommandBuffer BeginSingleTimeCommand(VkCommandPool CommandPool, VkDevice LogicalDevice);

	void EndSingleTimeCommand(VkCommandBuffer CommandBuffer, VkCommandPool CommandPool, VkDevice LogicalDevice, VkQueue GraphicsQueue);

#pragma endregion

#pragma region Image Creation
	struct GPUResource {
		VkImage texture_image;
		VkDeviceMemory texture_image_memory;
		VkImageView texture_image_view;
	};

	struct CreateImageContext {
		VkDevice logical_device;
		VkPhysicalDevice physical_device;
		uint32_t width;
		uint32_t height;
		VkFormat format;
		VkImageTiling tiling;
		VkImageUsageFlags usage_flags;
		VkMemoryPropertyFlags required_properties;
	};
	struct GPUImage {
		VkImage texture_image;
		VkDeviceMemory texture_image_memory;
	};
	GPUImage CreateImage(const CreateImageContext& Context);

	struct ImageViewContext {
		VkDevice logical_device;
		VkImage image;
		VkFormat image_format;
		VkImageAspectFlags aspect_flags;
	};
	VkImageView CreateImageView(const ImageViewContext& Context);
#pragma endregion

}