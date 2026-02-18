#pragma once

#include <vulkan/vulkan.hpp>
#include "VkCommon.h"

namespace renderer::data {

	struct Buffer {
		VkBuffer Buffer;
		VkDeviceMemory Memory;
		uint64_t ByteSize = 0;
	};
	struct BaseBufferContext {
		VkDevice LogicalDevice;
		VkPhysicalDevice PhysicalDevice;
		VkQueue GraphicsQueue;
		VkCommandPool CommandPool;
	};
	Buffer CreateBuffer(const void* Data, VkDeviceSize DataSize, VkBufferUsageFlags Usage, BaseBufferContext Base);
	void DestroyBuffer(VkDevice LogicalDevice, Buffer& Instance);

	struct UBO {
		Buffer Buffer;
		void* BufferMapped;
	};
	UBO CreateUBO(VkDevice LogicalDevice, VkPhysicalDevice PhysicalDevice, uint16_t SizeAllocated);
	void DestroyUBO(VkDevice LogicalDevice, UBO& Instance);

	void UpdateDescriptorSets(
		std::vector<VkDescriptorSet>& DescriptorSet,
		VkDevice LogicalDevice, 
		Buffer InstanceData, 
		Buffer BoundingBoxData,
		std::array<data::UBO, MAX_FRAMES_IN_FLIGHT> UniformBuffers,
		std::array<data::Buffer, MAX_FRAMES_IN_FLIGHT> ShouldDrawFlagBuffers);
}