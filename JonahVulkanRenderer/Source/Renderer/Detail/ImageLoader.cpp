#include <iostream>
#include "RendererDetail.h"

// Unnamed namespace to show functions below are pure Utility strictly for this .cpp file.
namespace {

	// Function waits for a specific part of the pipeline to transfer a Image layout
	void TransitionImageLayout(VkQueue GraphicsQueue, VkDevice LogicalDevice, VkCommandPool CommandPool, VkImage Image, VkFormat Format, VkImageLayout OldLayout, VkImageLayout NewLayout) {

		VkCommandBuffer command_buffer = renderer::detail::BeginSingleTimeCommand(CommandPool, LogicalDevice);

		VkPipelineStageFlags source_stage;
		VkPipelineStageFlags destination_stage;

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

		if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && NewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (OldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else {
			throw std::invalid_argument("Unsupported layout transition.");
		}

		vkCmdPipelineBarrier(command_buffer, source_stage, destination_stage, 0, 0, nullptr, 0, nullptr, 1, &barrier); // TODO

		renderer::detail::EndSingleTimeCommand(command_buffer, CommandPool, LogicalDevice, GraphicsQueue);
	}

	void CopyBufferToImage(VkQueue GraphicsQueue, VkDevice LogicalDevice, VkCommandPool CommandPool, VkBuffer Buffer, VkImage Image, uint32_t Width, uint32_t Height) {

		VkCommandBuffer command_buffer = renderer::detail::BeginSingleTimeCommand(CommandPool, LogicalDevice);

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0,0,0 };
		region.imageExtent = {
			Width, Height, 1
		};

		vkCmdCopyBufferToImage(command_buffer, Buffer, Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		renderer::detail::EndSingleTimeCommand(command_buffer, CommandPool, LogicalDevice, GraphicsQueue);

	}
}

// Implements all Image Loading functions in "RendererDetail.h" to be used in "Renderer.cpp"
namespace renderer::detail {

	TextureBundle LoadTextureImage(const char* TexturePath) {

		TextureBundle texture_loaded;

		int texture_channels;
		texture_loaded.pixels = stbi_load(TexturePath, &texture_loaded.width, &texture_loaded.height, &texture_channels, STBI_rgb_alpha);
		texture_loaded.image_size = texture_loaded.width * texture_loaded.height * 4;
		texture_loaded.format = VK_FORMAT_R8G8B8A8_SRGB;

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
		BufferData staging_buffer = detail::CreateDataBuffer(Context.logical_device, Context.physical_device, Context.texture_bundle.image_size, usage_flags, property_flags);

		// Copy image data to our staging buffer
		void* data;
		vkMapMemory(Context.logical_device, staging_buffer.memory_allocated_for_buffer, 0, Context.texture_bundle.image_size, 0, &data);
		memcpy(data, Context.texture_bundle.pixels, static_cast<size_t>(Context.texture_bundle.image_size));
		vkUnmapMemory(Context.logical_device, staging_buffer.memory_allocated_for_buffer);

		// Create image on GPU
		ImageObject new_image{};

		uint32_t width = static_cast<uint32_t>(Context.texture_bundle.width);
		uint32_t height = static_cast<uint32_t>(Context.texture_bundle.height);

		VkImageCreateInfo image_info{};
		image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image_info.imageType = VK_IMAGE_TYPE_2D;
		image_info.extent.width = width;
		image_info.extent.height = height;
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

		// Pass Buffer data into Image
		VkImageLayout old_layout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkImageLayout new_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		TransitionImageLayout(Context.graphics_queue, Context.logical_device, Context.command_pool, new_image.texture_image, Context.texture_bundle.format, old_layout, new_layout);

		CopyBufferToImage(Context.graphics_queue, Context.logical_device, Context.command_pool, staging_buffer.created_buffer, new_image.texture_image, width, height);

		old_layout = new_layout;
		new_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		TransitionImageLayout(Context.graphics_queue, Context.logical_device, Context.command_pool, new_image.texture_image, Context.texture_bundle.format, old_layout, new_layout);

		vkDestroyBuffer(Context.logical_device, staging_buffer.created_buffer, nullptr);
		vkFreeMemory(Context.logical_device, staging_buffer.memory_allocated_for_buffer, nullptr);

		return new_image;
	}
}