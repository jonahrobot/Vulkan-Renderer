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

	/*
	 * Optimization:
	 * Instead of creating a VkBuffer on the GPU and writing to it from the CPU, we create two buffers.
	 * One opens the CPU to write data onto the GPU.
	 * The second is strictly for the GPU.
	 *
	 * The first is a temp buffer, whose data is copied to the GPU only buffer.
	 * This allows us to end with a buffer only on the GPU. Reducing read times as CPU coherency is not required.
	 */

	renderer::detail::BufferData CreateGPULocalBuffer(const void* DataSrc, VkDeviceSize DataSize, VkBufferUsageFlags UsageFlags, VkDevice LogicalDevice, VkPhysicalDevice PhysicalDevice, VkQueue GraphicsQueue, VkCommandPool CommandPool) {

		// Create temp buffer that CPU and GPU can both see and write to.
		VkBufferUsageFlags temp_usage_flags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		VkMemoryPropertyFlags temp_property_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		renderer::detail::BufferData temp_buffer_data = CreateDataBuffer(LogicalDevice, PhysicalDevice, DataSize, temp_usage_flags, temp_property_flags);
		VkBuffer temp_buffer = temp_buffer_data.created_buffer;
		VkDeviceMemory temp_buffer_memory = temp_buffer_data.memory_allocated_for_buffer;

		void* data;
		vkMapMemory(LogicalDevice, temp_buffer_memory, 0, DataSize, 0, &data);
		memcpy(data, DataSrc, (size_t)DataSize);
		vkUnmapMemory(LogicalDevice, temp_buffer_memory);

		// Create GPU only buffer that CPU can't see
		VkBufferUsageFlags usage_flags = UsageFlags;
		VkMemoryPropertyFlags property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		renderer::detail::BufferData created_buffer_data = CreateDataBuffer(LogicalDevice, PhysicalDevice, DataSize, usage_flags, property_flags);

		// Copy temp buffer data into GPU only buffer
		CopyBuffer(GraphicsQueue, LogicalDevice, CommandPool, temp_buffer, created_buffer_data.created_buffer, DataSize);

		// Destroy temp
		vkDestroyBuffer(LogicalDevice, temp_buffer, nullptr);
		vkFreeMemory(LogicalDevice, temp_buffer_memory, nullptr);

		return created_buffer_data;
	}

}


// Implements all Vertex Buffer functions in "RendererDetail.h" to be used in "Renderer.cpp"
namespace renderer::detail {
	

	BufferData CreateVertexBuffer(const VertexBufferContext& Context) {

		const void* data_src = Context.vertices_to_render.data();
		VkDeviceSize data_size = sizeof(Context.vertices_to_render[0]) * Context.vertices_to_render.size();
		VkBufferUsageFlags usage_flags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

		BufferData created_buffer = CreateGPULocalBuffer(data_src, data_size, usage_flags, Context.logical_device,
										   Context.physical_device, Context.graphics_queue, Context.command_pool);

		return created_buffer;
	}

	BufferData CreateIndexBuffer(const IndexBufferContext& Context) {

		const void* data_src = Context.indices.data();
		VkDeviceSize data_size = sizeof(Context.indices[0]) * Context.indices.size();
		VkBufferUsageFlags usage_flags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

		BufferData created_buffer = CreateGPULocalBuffer(data_src, data_size, usage_flags, Context.logical_device,
										   Context.physical_device, Context.graphics_queue, Context.command_pool);

		return created_buffer;

	}

	UniformBufferData CreateUniformBuffers(const UniformBufferContext& Context) {

		UniformBufferData buffer_data = {};
		buffer_data.uniform_buffers.resize(Context.max_frames_in_flight);
		buffer_data.uniform_buffers_memory.resize(Context.max_frames_in_flight);
		buffer_data.uniform_buffers_mapped.resize(Context.max_frames_in_flight);

		for (size_t i = 0; i < Context.max_frames_in_flight; i++) {
			VkBufferUsageFlags usage_flags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			VkMemoryPropertyFlags property_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			BufferData current_buffer = CreateDataBuffer(Context.logical_device, Context.physical_device, Context.ubo_size, usage_flags, property_flags);
		
			buffer_data.uniform_buffers[i] = current_buffer.created_buffer;
			buffer_data.uniform_buffers_memory[i] = current_buffer.memory_allocated_for_buffer;

			vkMapMemory(Context.logical_device, buffer_data.uniform_buffers_memory[i], 0, Context.ubo_size, 0, &buffer_data.uniform_buffers_mapped[i]);
		}

		return buffer_data;
	}
}