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

	struct BufferData {
		VkBuffer created_buffer;
		VkDeviceMemory memory_allocated_for_buffer;
	};
	BufferData CreateDataBuffer(VkDevice LogicalDevice, VkPhysicalDevice PhysicalDevice, VkDeviceSize BufferSize, VkBufferUsageFlags UsageFlags, VkMemoryPropertyFlags PropertyFlags) {

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

		BufferData return_data{};
		return_data.created_buffer = buffer;
		return_data.memory_allocated_for_buffer = buffer_memory;

		return return_data;
	}

	// Copy two buffers on the GPU through GPU commands.
	void CopyBuffer(VkQueue GraphicsQueue, VkDevice LogicalDevice, VkCommandPool CommandPool, VkBuffer SrcBuffer, VkBuffer DstBuffer, VkDeviceSize size) {
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

		VkBufferCopy copy_region{};
		copy_region.srcOffset = 0;
		copy_region.dstOffset = 0;
		copy_region.size = size;
		vkCmdCopyBuffer(command_buffer, SrcBuffer, DstBuffer, 1, &copy_region);

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


// Implements all Vertex Buffer functions in "RendererDetail.h" to be used in "Renderer.cpp"
namespace renderer::detail {
	
	/*
	 * Optimization: 
	 * Instead of creating a VkBuffer on the GPU and writing to it from the CPU, we create two buffers.
	 * One opens the CPU to write data onto the GPU.
	 * The second is strictly for the GPU.
	 * 
	 * The first is a temp buffer, whose data is copied to the GPU only buffer.
	 * This allows us to end with a buffer only on the GPU. Reducing read times as CPU coherency is not required.
	 */

	VertexBufferData CreateVertexBuffer(const VertexBufferContext& Context) {

		VkDeviceSize vertex_buffer_size = sizeof(Context.vertices_to_render[0]) * Context.vertices_to_render.size();

		// Create temp buffer that CPU and GPU can both see and write to.
		VkBufferUsageFlags temp_usage_flags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		VkMemoryPropertyFlags temp_property_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		BufferData temp_buffer_data = CreateDataBuffer(Context.logical_device, Context.physical_device, vertex_buffer_size, temp_usage_flags, temp_property_flags);
		VkBuffer temp_buffer = temp_buffer_data.created_buffer;
		VkDeviceMemory temp_buffer_memory = temp_buffer_data.memory_allocated_for_buffer;

		void* data;
		vkMapMemory(Context.logical_device, temp_buffer_memory, 0, vertex_buffer_size, 0, &data);
		memcpy(data, Context.vertices_to_render.data(), (size_t)vertex_buffer_size);
		vkUnmapMemory(Context.logical_device, temp_buffer_memory);

		// Create GPU only buffer that CPU can't see
		VkBufferUsageFlags usage_flags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		VkMemoryPropertyFlags property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		BufferData vertex_buffer_data = CreateDataBuffer(Context.logical_device, Context.physical_device, vertex_buffer_size, usage_flags, property_flags);
		VkBuffer vertex_buffer = vertex_buffer_data.created_buffer;

		// Copy temp buffer data into GPU only buffer
		CopyBuffer(Context.graphics_queue, Context.logical_device, Context.command_pool, temp_buffer, vertex_buffer, vertex_buffer_size);

		// Destroy temp
		vkDestroyBuffer(Context.logical_device, temp_buffer, nullptr);
		vkFreeMemory(Context.logical_device, temp_buffer_memory, nullptr);

		VertexBufferData return_data{};
		return_data.created_buffer = vertex_buffer;
		return_data.memory_allocated_for_buffer = vertex_buffer_data.memory_allocated_for_buffer;

		return return_data;
	}

}