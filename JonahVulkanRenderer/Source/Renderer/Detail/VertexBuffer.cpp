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
}


// Implements all Vertex Buffer functions in "RendererDetail.h" to be used in "Renderer.cpp"
namespace renderer::detail {
	
	VkBuffer CreateVertexBuffer(const VertexBufferContext& Context) {

		VkBuffer vertex_buffer;

		VkBufferCreateInfo buffer_info{};
		buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_info.size = Context.buffer_size;
		buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(Context.logical_device, &buffer_info, nullptr, &vertex_buffer) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create vertex buffer.");
		}

		return vertex_buffer;
	}

	VkDeviceMemory AllocateVertexBuffer(const AllocateMemoryContext& Context) {

		VkDeviceMemory vertex_buffer_memory;

		VkMemoryRequirements memory_requirements;
		vkGetBufferMemoryRequirements(Context.logical_device, Context.vertex_buffer, &memory_requirements);

		VkMemoryPropertyFlags memory_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		VkMemoryAllocateInfo allocate_info{};
		allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocate_info.allocationSize = memory_requirements.size;
		allocate_info.memoryTypeIndex = FindMemoryType(Context.physical_device, memory_requirements.memoryTypeBits, memory_flags);

		if (vkAllocateMemory(Context.logical_device, &allocate_info, nullptr, &vertex_buffer_memory) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate vertex buffer memory.");
		}

		return vertex_buffer_memory;
	}
}