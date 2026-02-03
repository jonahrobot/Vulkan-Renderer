#include "RendererDetail.h"
#include <set>
#include <iostream>

// Implements all Vulkan Logical Device Creation functions in "RendererDetail.h" to be used in "Renderer.cpp"
namespace renderer::detail {

	VulkanCore CreateLogicalDevice(const PhysicalDevice_Stage& Context, bool UseValidationLayers, const std::vector<const char*>& ValidationLayersToSupport, const std::vector<const char*>& DeviceExtensionsToSupport){
		
		VkDevice logical_device;

		std::vector<VkDeviceQueueCreateInfo> queue_create_infos;

		// We convert supported queues to a set for easier iteration
		uint32_t graphics_index = Context.queues_supported.graphics_compute_family.value();
		uint32_t present_index = Context.queues_supported.present_family.value();
		std::set<uint32_t> supported_queues = { graphics_index, present_index };

		float queue_priority = 1.0f;
		for (uint32_t queue : supported_queues) {

			VkDeviceQueueCreateInfo create_info{};
			create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			create_info.queueFamilyIndex = queue;
			create_info.queueCount = 1;
			create_info.pQueuePriorities = &queue_priority;

			queue_create_infos.push_back(create_info);
		}

		VkPhysicalDeviceFeatures device_features{};
		device_features.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = queue_create_infos.data();
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
		createInfo.pEnabledFeatures = &device_features;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(DeviceExtensionsToSupport.size());;
		createInfo.ppEnabledExtensionNames = DeviceExtensionsToSupport.data();

		// New versions of vulkan merge device extension validation with validation layers.
		if (UseValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayersToSupport.size());
			createInfo.ppEnabledLayerNames = ValidationLayersToSupport.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		VkResult create_device_successful = vkCreateDevice(Context.physical_device, &createInfo, nullptr, &logical_device);

		if (create_device_successful != VK_SUCCESS) {
			throw std::runtime_error("Failed to create logical device!");
		}

		return {Context.vulkan_instance, Context.window, Context.vulkan_surface, Context.physical_device, Context.queues_supported, logical_device};
	}

} // namespace renderer::detail