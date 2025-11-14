#include <iostream>
#include "RendererDetail.h"

// Unnamed namespace to show functions below are pure Utility strictly for this .cpp file.
namespace {

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

	renderer::detail::BufferData CreateDataBuffer(VkDevice LogicalDevice, VkPhysicalDevice PhysicalDevice, VkDeviceSize BufferSize, VkBufferUsageFlags UsageFlags, VkMemoryPropertyFlags PropertyFlags) {

		VkBuffer buffer;

		// Create buffer
		VkBufferCreateInfo buffer_info{};
		buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_info.size = BufferSize;
		buffer_info.usage = UsageFlags;
		buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(LogicalDevice, &buffer_info, nullptr, &buffer) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create vertex buffer.");
		}

		// Allocate buffer memory
		VkDeviceMemory buffer_memory;

		VkMemoryRequirements memory_requirements;
		vkGetBufferMemoryRequirements(LogicalDevice, buffer, &memory_requirements);

		VkMemoryAllocateInfo allocate_info{};
		allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocate_info.allocationSize = memory_requirements.size;
		allocate_info.memoryTypeIndex = FindMemoryType(PhysicalDevice, memory_requirements.memoryTypeBits, PropertyFlags);

		if (vkAllocateMemory(LogicalDevice, &allocate_info, nullptr, &buffer_memory) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate vertex buffer memory.");
		}

		vkBindBufferMemory(LogicalDevice, buffer, buffer_memory, 0);

		renderer::detail::BufferData return_data{};
		return_data.created_buffer = buffer;
		return_data.memory_allocated_for_buffer = buffer_memory;

		return return_data;
	}

	void TransitionImageLayout(VkQueue GraphicsQueue, VkDevice LogicalDevice, VkCommandPool CommandPool, VkImage Image, VkFormat Format, VkImageLayout OldLayout, VkImageLayout NewLayout) {

		// Start single time command

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

		// 
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = OldLayout;
		barrier.newLayout = NewLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = Image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = 0;

		vkCmdPipelineBarrier(command_buffer, 0, 0, 0, 0, nullptr, 0, nullptr, 1, &barrier); // TODO

		// End single time command

		vkEndCommandBuffer(command_buffer);

		VkSubmitInfo submit_info{};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &command_buffer;

		vkQueueSubmit(GraphicsQueue, 1, &submit_info, VK_NULL_HANDLE);
		vkQueueWaitIdle(GraphicsQueue);

		vkFreeCommandBuffers(LogicalDevice, CommandPool, 1, &command_buffer);
	}
}

// Implements all Image Loading functions in "RendererDetail.h" to be used in "Renderer.cpp"
namespace renderer::detail {

	TextureBundle LoadTextureImage(const char* TexturePath) {

		TextureBundle texture_loaded;

		int texture_channels;
		texture_loaded.pixels = stbi_load(TexturePath, &texture_loaded.width, &texture_loaded.height, &texture_channels, STBI_rgb_alpha);
		texture_loaded.image_size = texture_loaded.width * texture_loaded.height * 4;

		if (!texture_loaded.pixels) {
			throw std::runtime_error("Failed to load texture image.");
		}

		return texture_loaded;
	}

	void FreeTextureBundle(TextureBundle& TextureBundle) {
		stbi_image_free(TextureBundle.pixels);
		TextureBundle.pixels = NULL;
		TextureBundle.image_size = 0;
	}

	ImageObject CreateImageObject(const ImageObjectContext& Context) {

		// Create staging buffer
		VkBufferUsageFlags usage_flags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		VkMemoryPropertyFlags property_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		BufferData staging_buffer = CreateDataBuffer(Context.logical_device, Context.physical_device, Context.texture_bundle.image_size, usage_flags, property_flags);

		// Copy image data to our staging buffer
		void* data;
		vkMapMemory(Context.logical_device, staging_buffer.memory_allocated_for_buffer, 0, Context.texture_bundle.image_size, 0, &data);
		memcpy(data, Context.texture_bundle.pixels, static_cast<size_t>(Context.texture_bundle.image_size));
		vkUnmapMemory(Context.logical_device, staging_buffer.memory_allocated_for_buffer);

		// Create image on GPU
		ImageObject new_image{};

		VkImageCreateInfo image_info{};
		image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_info.imageType = VK_IMAGE_TYPE_2D;
		image_info.extent.width = static_cast<uint32_t>(Context.texture_bundle.width);
		image_info.extent.height = static_cast<uint32_t>(Context.texture_bundle.height);
		image_info.extent.depth = 1;
		image_info.mipLevels = 1;
		image_info.arrayLayers = 1;
		image_info.format = Context.texture_bundle.format;
		image_info.tiling = Context.data_tiling_mode;
		image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		image_info.usage = Context.usage_flags;
		image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		image_info.samples = VK_SAMPLE_COUNT_1_BIT;
		image_info.flags = 0;
		
		if (vkCreateImage(Context.logical_device, &image_info, nullptr, &new_image.texture_image) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create image.");
		}

		// Allocate memory for GPU Image
		VkMemoryRequirements mem_requirements;
		vkGetImageMemoryRequirements(Context.logical_device, new_image.texture_image, &mem_requirements);

		VkMemoryAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		alloc_info.allocationSize = mem_requirements.size;
		alloc_info.memoryTypeIndex = FindMemoryType(Context.physical_device,mem_requirements.memoryTypeBits, Context.memory_flags_required);

		if (vkAllocateMemory(Context.logical_device, &alloc_info, nullptr, &new_image.texture_image_memory) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate image memory.");
		}

		vkBindImageMemory(Context.logical_device, new_image.texture_image, new_image.texture_image_memory, 0);

		return new_image;
	}
}