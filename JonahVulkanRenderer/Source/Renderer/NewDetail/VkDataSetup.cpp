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

	void UpdateDescriptorSets(
		std::vector<VkDescriptorSet>& DescriptorSet,
		VkDevice LogicalDevice,
		Buffer InstanceData,
		Buffer MeshCenters,
		std::array<data::UBO, MAX_FRAMES_IN_FLIGHT> UniformBuffers,
		std::array<data::Buffer, MAX_FRAMES_IN_FLIGHT> ShouldDrawFlagBuffers,
		std::array<data::Buffer, MAX_FRAMES_IN_FLIGHT>  IndirectDrawBuffers){

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {

			// [0] Update Uniform Buffer Object
			VkDescriptorBufferInfo ubo_info{};
			ubo_info.buffer = UniformBuffers[i].Buffer.Buffer;
			ubo_info.offset = 0;
			ubo_info.range = UniformBuffers[i].Buffer.ByteSize;

			VkWriteDescriptorSet ubo = {};
			ubo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			ubo.dstSet = DescriptorSet[i];
			ubo.dstBinding = 0;
			ubo.dstArrayElement = 0;
			ubo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			ubo.descriptorCount = 1;
			ubo.pBufferInfo = &ubo_info;

			// [1] Update Instance Data SSBO
			VkDescriptorBufferInfo instance_data_info{};
			instance_data_info.buffer = InstanceData.Buffer;
			instance_data_info.offset = 0;
			instance_data_info.range = InstanceData.ByteSize;

			VkWriteDescriptorSet instance_data = {};
			instance_data.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			instance_data.dstSet = DescriptorSet[i];
			instance_data.dstBinding = 1;
			instance_data.dstArrayElement = 0;
			instance_data.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			instance_data.descriptorCount = 1;
			instance_data.pBufferInfo = &instance_data_info;

			// [2] Update Indirect Draw Command SSBO
			VkDescriptorBufferInfo draw_command_info{};
			draw_command_info.buffer = IndirectDrawBuffers[i].Buffer;
			draw_command_info.offset = 0;
			draw_command_info.range = IndirectDrawBuffers[i].ByteSize;

			VkWriteDescriptorSet draw_commands = {};
			draw_commands.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			draw_commands.dstSet = DescriptorSet[i];
			draw_commands.dstBinding = 2;
			draw_commands.dstArrayElement = 0;
			draw_commands.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			draw_commands.descriptorCount = 1;
			draw_commands.pBufferInfo = &draw_command_info;

			// [3] Update Mesh Centers SSBO
			VkDescriptorBufferInfo mesh_centers_info{};
			mesh_centers_info.buffer = MeshCenters.Buffer;
			mesh_centers_info.offset = 0;
			mesh_centers_info.range = MeshCenters.ByteSize;

			VkWriteDescriptorSet mesh_centers = {};
			mesh_centers.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			mesh_centers.dstSet = DescriptorSet[i];
			mesh_centers.dstBinding = 3;
			mesh_centers.dstArrayElement = 0;
			mesh_centers.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			mesh_centers.descriptorCount = 1;
			mesh_centers.pBufferInfo = &mesh_centers_info;

			// [4] Update Should Draw Flags SSBO
			VkDescriptorBufferInfo should_draw_flags_info{};
			should_draw_flags_info.buffer = ShouldDrawFlagBuffers[i].Buffer;
			should_draw_flags_info.offset = 0;
			should_draw_flags_info.range = ShouldDrawFlagBuffers[i].ByteSize;

			VkWriteDescriptorSet should_draw_flags = {};
			should_draw_flags.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			should_draw_flags.dstSet = DescriptorSet[i];
			should_draw_flags.dstBinding = 4;
			should_draw_flags.dstArrayElement = 0;
			should_draw_flags.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			should_draw_flags.descriptorCount = 1;
			should_draw_flags.pBufferInfo = &should_draw_flags_info;

			std::array<VkWriteDescriptorSet, 5> descriptor_writes = {ubo, instance_data, draw_commands, mesh_centers, should_draw_flags};

			vkUpdateDescriptorSets(LogicalDevice, static_cast<uint32_t>(descriptor_writes.size()), descriptor_writes.data(), 0, nullptr);
		}

	}
}