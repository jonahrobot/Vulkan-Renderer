#include "RendererDetail.h"
#include <optional>
#include <set>
#include <string>
#include <iostream>

// Unnamed namespace to show functions below are pure Utility strictly for this .cpp file.
namespace {

#pragma region IsDeviceSuitable HELPERS

	renderer::detail::QueueFamilyIndices FindSupportedQueues(const VkPhysicalDevice physical_device, const VkSurfaceKHR current_surface) {
		
		renderer::detail::QueueFamilyIndices queues_found;

		uint32_t supported_queues_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &supported_queues_count, nullptr);
		std::vector<VkQueueFamilyProperties> queues_supported_by_device(supported_queues_count);
		vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &supported_queues_count, queues_supported_by_device.data());

		int index = 0;
		for (const VkQueueFamilyProperties& queue : queues_supported_by_device) {

			if (queue.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				queues_found.graphicsFamily = index;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, index, current_surface, &presentSupport);

			if (presentSupport) {
				queues_found.presentFamily = index;
			}

			if (queues_found.isComplete()) {
				break;
			}

			index++;
		}

		return queues_found;
	}
	
	bool CheckDeviceExtensionSupport(const VkPhysicalDevice physical_device, const std::vector<const char*>& deviceExtensionsToSupport) {

		uint32_t available_extension_count;
		vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &available_extension_count, nullptr);
		std::vector<VkExtensionProperties> available_extensions(available_extension_count);
		vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &available_extension_count, available_extensions.data());

		std::set<std::string> required_extensions(deviceExtensionsToSupport.begin(), deviceExtensionsToSupport.end());

		for (const VkExtensionProperties& extension : available_extensions) {
			required_extensions.erase(extension.extensionName);
		}

		return required_extensions.empty();
	}

#pragma endregion

	bool IsDeviceSuitable(const VkPhysicalDevice physical_device,const VkSurfaceKHR current_surface, const std::vector<const char*>& deviceExtensionsToSupport) {

		renderer::detail::QueueFamilyIndices queues_found = FindSupportedQueues(physical_device, current_surface);

		bool all_extensions_supported = CheckDeviceExtensionSupport(physical_device, deviceExtensionsToSupport);

		bool swap_chain_capable = false;

		// Only check swapchain capabilities if GPU supports swapchains.
		if (all_extensions_supported) { 
			renderer::detail::SwapChainSupportDetails swap_chain_details = renderer::detail::GetDeviceSwapchainSupport(physical_device, current_surface);
			swap_chain_capable = !swap_chain_details.formats.empty() && !swap_chain_details.presentModes.empty();
		}

		return queues_found.isComplete() && all_extensions_supported && swap_chain_capable;
	}

}


// Implements all Vulkan Physical Device Picking functions in "RendererDetail.h" to be used in "Renderer.cpp"
namespace renderer::detail {

	PhysicalDeviceData PickPhysicalDevice(const PhysicalDeviceContext& Context) {

		VkPhysicalDevice physical_device = VK_NULL_HANDLE;

		uint32_t device_count = 0;
		vkEnumeratePhysicalDevices(Context.vulkan_instance, &device_count, nullptr);
		if (device_count == 0) {
			throw std::runtime_error("Failed to find GPUs with Vulkan support!");
		}
		std::vector<VkPhysicalDevice> devices(device_count);
		vkEnumeratePhysicalDevices(Context.vulkan_instance, &device_count, devices.data());

		for (const VkPhysicalDevice& device : devices) {
			if (IsDeviceSuitable(device, Context.vulkan_surface, Context.DeviceExtensionsToSupport)) {
				physical_device = device;
				break;
			}
		}

		if (physical_device == VK_NULL_HANDLE) {
			throw std::runtime_error("Failed to find a suitable GPU.");
		}

		PhysicalDeviceData return_data{};

		return_data.physical_device = physical_device;
		return_data.queues_supported = FindSupportedQueues(physical_device, Context.vulkan_surface);

		return return_data;
	}

	SwapChainSupportDetails GetDeviceSwapchainSupport(const VkPhysicalDevice physical_device, const VkSurfaceKHR current_surface) {

		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, current_surface, &details.capabilities);

		uint32_t supported_format_count;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, current_surface, &supported_format_count, nullptr);
		if (supported_format_count != 0) {
			details.formats.resize(supported_format_count);
			vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, current_surface, &supported_format_count, details.formats.data());
		}

		uint32_t supported_present_mode_count;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, current_surface, &supported_present_mode_count, nullptr);
		if (supported_present_mode_count != 0) {
			details.presentModes.resize(supported_present_mode_count);
			vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, current_surface, &supported_present_mode_count, details.presentModes.data());
		}

		return details;
	}
} // namespace renderer::detail