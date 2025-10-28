#include <iostream>
#include "RendererDetail.h"

// Unnamed namespace to show functions below are pure Utility strictly for this .cpp file.
namespace {
}


// Implements all Syncronization Primative functions in "RendererDetail.h" to be used in "Renderer.cpp"
namespace renderer::detail {

	VkSemaphore CreateVulkanSemaphore(VkDevice LogicalDevice) {
		VkSemaphore semaphore;
		VkSemaphoreCreateInfo semaphore_info{};
		semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		if (vkCreateSemaphore(LogicalDevice, &semaphore_info, nullptr, &semaphore) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create semaphore.");
		}
		return semaphore;
	}

	VkFence CreateVulkanFence(VkDevice LogicalDevice) {
		VkFence fence;
		VkFenceCreateInfo fence_info{};
		fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		if (vkCreateFence(LogicalDevice, &fence_info, nullptr, &fence) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create fence.");
		}
		return fence;
	}

}