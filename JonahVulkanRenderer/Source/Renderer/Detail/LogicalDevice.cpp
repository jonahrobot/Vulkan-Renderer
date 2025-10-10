#include "RendererDetail.h"
#include <set>
#include <iostream>

// Implements all Vulkan Logical Device Creation functions in "RendererDetail.h" to be used in "Renderer.cpp"
namespace renderer::detail {

	VkDevice CreateLogicalDevice(const LogicalDeviceContext& Context) {
		
		VkDevice logical_device;

		std::vector<VkDeviceQueueCreateInfo> queue_create_infos;

		// We convert supported queues to a set for easier iteration
		uint32_t graphics_index = Context.supported_queues.graphicsFamily.value();
		uint32_t present_index = Context.supported_queues.presentFamily.value();
		std::set<uint32_t> supported_queues = { graphics_index, present_index };

		float queue_priority = 1.0f;
		for (uint32_t queue : supported_queues) {

			VkDeviceQueueCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			createInfo.queueFamilyIndex = queue;
			createInfo.queueCount = 1;
			createInfo.pQueuePriorities = &queue_priority;

			queue_create_infos.push_back(createInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = queue_create_infos.data();
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(Context.DeviceExtensionsToSupport.size());;
		createInfo.ppEnabledExtensionNames = Context.DeviceExtensionsToSupport.data();

		// New versions of vulkan merge device extension validation with validation layers.
		if (Context.UseValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(Context.ValidationLayersToSupport.size());
			createInfo.ppEnabledLayerNames = Context.ValidationLayersToSupport.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		VkResult create_device_successful = vkCreateDevice(Context.physical_device, &createInfo, nullptr, &logical_device);

		if (create_device_successful != VK_SUCCESS) {
			throw std::runtime_error("Failed to create logical device!");
		}

		return logical_device;
	}

} // namespace renderer::detail