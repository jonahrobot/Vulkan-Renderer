#include "RendererDetail_Common.h"
#include <iostream>

namespace renderer::detail {

	MergedIndexVertexBuffer MergeIndexVertexBuffer(const std::vector<Vertex>& V1, const std::vector<uint32_t>& I1, const std::vector<Vertex>& V2, const std::vector<uint32_t>& I2) {

		MergedIndexVertexBuffer MergedData{};

		MergedData.merged_vertex_buffer = V1;
		for (Vertex point : V2) {
			point.position.z += 1;
			MergedData.merged_vertex_buffer.push_back(point);
		}

		MergedData.merged_index_buffer = I1;
		uint32_t size_of_v1 = static_cast<uint32_t>(V1.size());
		for (uint32_t x : I2) {
			MergedData.merged_index_buffer.push_back(x + size_of_v1);
		}

		return MergedData;
	}

	uint32_t FindMemoryType(VkPhysicalDevice PhysicalDevice, uint32_t TypeFilter, VkMemoryPropertyFlags properties) {

		VkPhysicalDeviceMemoryProperties memory_properties;
		vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &memory_properties);

		for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {
			uint8_t check_one = TypeFilter & (1 << i);
			uint8_t check_two = memory_properties.memoryTypes[i].propertyFlags & properties;
			if (check_one && check_two == properties) {
				return i;
			}
		}

		throw std::runtime_error("Failed to find suitable memory type.");
	}

	VkCommandBuffer BeginSingleTimeCommand(VkCommandPool CommandPool, VkDevice LogicalDevice) {
		VkCommandBufferAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandPool = CommandPool;
		alloc_info.commandBufferCount = 1;

		VkCommandBuffer command_buffer;
		vkAllocateCommandBuffers(LogicalDevice, &alloc_info, &command_buffer);

		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(command_buffer, &begin_info);

		return command_buffer;
	}

	void EndSingleTimeCommand(VkCommandBuffer CommandBuffer, VkCommandPool CommandPool, VkDevice LogicalDevice, VkQueue GraphicsQueue) {
		vkEndCommandBuffer(CommandBuffer);

		VkSubmitInfo submit_info{};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &CommandBuffer;

		vkQueueSubmit(GraphicsQueue, 1, &submit_info, VK_NULL_HANDLE);
		vkQueueWaitIdle(GraphicsQueue);

		vkFreeCommandBuffers(LogicalDevice, CommandPool, 1, &CommandBuffer);
	}

	GPUImage CreateImage(const CreateImageContext& Context) {

		GPUImage our_image{};

		VkImageCreateInfo image_info{};
		image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_info.imageType = VK_IMAGE_TYPE_2D;
		image_info.extent.width = Context.width;
		image_info.extent.height = Context.height;
		image_info.extent.depth = 1;
		image_info.mipLevels = 1;
		image_info.arrayLayers = Context.array_layers;
		image_info.format = Context.format;
		image_info.tiling = Context.tiling;
		image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		image_info.usage = Context.usage_flags;
		image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		image_info.samples = VK_SAMPLE_COUNT_1_BIT;
		image_info.flags = 0;

		if (vkCreateImage(Context.logical_device, &image_info, nullptr, &our_image.image) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create image.");
		}

		// Allocate memory for GPU Image
		VkMemoryRequirements mem_requirements;
		vkGetImageMemoryRequirements(Context.logical_device, our_image.image, &mem_requirements);

		VkMemoryAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		alloc_info.allocationSize = mem_requirements.size;
		alloc_info.memoryTypeIndex = FindMemoryType(Context.physical_device, mem_requirements.memoryTypeBits, Context.required_properties);

		if (vkAllocateMemory(Context.logical_device, &alloc_info, nullptr, &our_image.image_memory) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate image memory.");
		}

		vkBindImageMemory(Context.logical_device, our_image.image, our_image.image_memory, 0);

		return our_image;
	}

	VkImageView CreateImageView(const ImageViewContext& Context) {

		VkImageView our_view;

		VkImageViewCreateInfo view_info{};
		view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view_info.image = Context.image;
		view_info.viewType = Context.view_type;
		view_info.format = Context.image_format;
		view_info.subresourceRange.aspectMask = Context.aspect_flags;
		view_info.subresourceRange.baseMipLevel = 0;
		view_info.subresourceRange.levelCount = 1;
		view_info.subresourceRange.baseArrayLayer = 0;
		view_info.subresourceRange.layerCount = Context.array_layers;

		if (vkCreateImageView(Context.logical_device, &view_info, nullptr, &our_view) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create image view.");
		}
		
		return our_view;
	}
}