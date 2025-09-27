#include "RendererDetail.h"
#include <set>
#include <iostream>

// Implements all Vulkan Logical Device Creation functions in "RendererDetail.h" to be used in "Renderer.cpp"
namespace renderer::detail {

	VkDevice CreateLogicalDevice(const LogicalDeviceContext& context, const std::vector<const char*>& deviceExtensionsToSupport, const std::vector<const char*>& ValidationLayersToSupport) {
		
		VkDevice logical_device;

		// We convert supported queues to a set for easier iteration
		std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
		uint32_t graphics_index = context.supported_queues.graphicsFamily.value();
		uint32_t present_index = context.supported_queues.presentFamily.value();
		std::set<uint32_t> supported_queues_set = { graphics_index, present_index };

		float queue_priority = 1.0f;
		for (uint32_t queue : supported_queues_set) {

			VkDeviceQueueCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			createInfo.queueFamilyIndex = queue;
			createInfo.queueCount = 1;
			createInfo.pQueuePriorities = &queue_priority;

			queue_create_infos.push_back(createInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};

		// Create Logical Device
		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = queue_create_infos.data();
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensionsToSupport.size());;
		createInfo.ppEnabledExtensionNames = deviceExtensionsToSupport.data();

		// Support device specific validation layers
		// New versions of vulkan instance validation layers also handle device validations
		if (context.UseValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayersToSupport.size());
			createInfo.ppEnabledLayerNames = ValidationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(context.physical_device, &createInfo, nullptr, &logical_device) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create logical device!");
		}

		return logical_device;
	}


} // namespace renderer::detail