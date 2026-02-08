#include "VkDataSetup.h"
#include "VkCommon.h"

namespace {

	uint32_t FindMemoryType(VkPhysicalDevice PhysicalDevice, uint32_t TypeFilter, VkMemoryPropertyFlags Properties) {

		VkPhysicalDeviceMemoryProperties memory_properties;
		vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &memory_properties);

		for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {
			uint8_t check_one = TypeFilter & (1 << i);
			uint8_t check_two = memory_properties.memoryTypes[i].propertyFlags & Properties;
			if (check_one && check_two == Properties) {
				return i;
			}
		}

		throw std::runtime_error("Failed to find suitable memory type.");
	}

	renderer::data::Buffer CreateBufferHelper(
		VkDevice LogicalDevice, 
		VkPhysicalDevice PhysicalDevice, 
		VkDeviceSize BufferSize, 
		VkBufferUsageFlags UsageFlags, 
		VkMemoryPropertyFlags PropertyFlags
	){

		// Create VkBuffer
		VkBufferCreateInfo buffer_info{};
		buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_info.size = BufferSize;
		buffer_info.usage = UsageFlags;
		buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkBuffer buffer;
		if (vkCreateBuffer(LogicalDevice, &buffer_info, nullptr, &buffer) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create vertex buffer.");
		}

		// Create VkDeviceMemory
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

		renderer::data::Buffer buffer_struct{};
		buffer_struct.Buffer = buffer;
		buffer_struct.Memory = buffer_memory;
		buffer_struct.ByteSize = BufferSize;

		return buffer_struct;
	}
}

namespace renderer::data {

	UBO CreateUBO(VkDevice LogicalDevice, VkPhysicalDevice PhysicalDevice, uint16_t SizeAllocated) {
		UBO ubo;

		if (SizeAllocated == 0) return ubo;

		VkBufferUsageFlags usage_flags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		VkMemoryPropertyFlags property_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		ubo.Buffer = CreateBufferHelper(LogicalDevice, PhysicalDevice, SizeAllocated, usage_flags, property_flags);

		vkMapMemory(LogicalDevice, ubo.Buffer.Memory, 0, SizeAllocated, 0, &ubo.BufferMapped);

		return ubo;
	}

	void DestroyUBO(VkDevice LogicalDevice, UBO& Instance) {
		DestroyBuffer(LogicalDevice, Instance.Buffer);
	}

	Buffer CreateBuffer(const void* Data, VkDeviceSize DataSize, VkBufferUsageFlags Usage, BaseBufferContext Base) {

		if (DataSize == 0) return Buffer{};

		// Create temp buffer
		VkBufferUsageFlags temp_usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		VkMemoryPropertyFlags temp_properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		Buffer temp_buffer = CreateBufferHelper(Base.LogicalDevice, Base.PhysicalDevice, DataSize, temp_usage, temp_properties);

		// Copy data -> temp buffer
		void* data;
		vkMapMemory(Base.LogicalDevice, temp_buffer.Memory, 0, DataSize, 0, &data);
		memcpy(data, Data, (size_t)DataSize);
		vkUnmapMemory(Base.LogicalDevice, temp_buffer.Memory);

		// Create final buffer
		Buffer buffer = CreateBufferHelper(Base.LogicalDevice, Base.PhysicalDevice, DataSize, Usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		// Copy temp buffer -> final buffer
		VkCommandBuffer command_buffer = BeginSingleTimeCommand(Base.CommandPool, Base.LogicalDevice);
		VkBufferCopy copy_region{.srcOffset = 0, .dstOffset = 0, .size = DataSize};
		vkCmdCopyBuffer(command_buffer, temp_buffer.Buffer, buffer.Buffer, 1, &copy_region);
		EndSingleTimeCommand(command_buffer, Base.CommandPool, Base.LogicalDevice, Base.GraphicsQueue);

		// Destroy temp
		DestroyBuffer(Base.LogicalDevice, temp_buffer);

		return buffer;
	}

	void DestroyBuffer(VkDevice LogicalDevice, Buffer& Instance) {
		if (Instance.Buffer == VK_NULL_HANDLE || Instance.Memory == VK_NULL_HANDLE) return;

		vkDestroyBuffer(LogicalDevice, Instance.Buffer, nullptr);
		vkFreeMemory(LogicalDevice, Instance.Memory, nullptr);
		Instance.Buffer = VK_NULL_HANDLE;
		Instance.Memory = VK_NULL_HANDLE;
		Instance.ByteSize = 0;
	}
}