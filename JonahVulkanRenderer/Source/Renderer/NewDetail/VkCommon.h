#pragma once

#include <vulkan/vulkan.hpp>
#include <optional>

namespace renderer {

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphics_compute_family;
		std::optional<uint32_t> present_family;
	};

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
}