#include <set>

#include "VkDeviceSetup.h"
#include "VkSwapchainSetup.h"
#include "VkCommon.h"

namespace {

	// Used by CreateVulkanInstance();
	bool CheckValidationLayerSupport(const std::vector<const char*>& ValidationLayersToSupport) {

		// Get all supported layers
		uint32_t layer_count;
		vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
		std::vector<VkLayerProperties> supported_layers(layer_count);
		vkEnumerateInstanceLayerProperties(&layer_count, supported_layers.data());

		// Ensure all wanted layers are supported
		for (const auto& required_layer : ValidationLayersToSupport) {
			bool found = false;

			for (const auto& available_layer : supported_layers) {
				if (strcmp(required_layer, available_layer.layerName) == 0) {
					found = true;
					break;
				}
			}

			if (!found) return false;
		}

		return true;
	}

	// Used by PickPhysicalDevice();
	bool IsDeviceSuitable(const VkPhysicalDevice PhysicalDevice, const VkSurfaceKHR CurrentSurface, const std::vector<const char*>& DeviceExtensionsToSupport) {

		// Conduct four checks to see if GPU (PhysicalDevice) will work with our renderer.

		// 1. Check if GPU supports all required Device Extensions
		uint32_t available_extension_count;
		vkEnumerateDeviceExtensionProperties(PhysicalDevice, nullptr, &available_extension_count, nullptr);
		std::vector<VkExtensionProperties> available_extensions(available_extension_count);
		vkEnumerateDeviceExtensionProperties(PhysicalDevice, nullptr, &available_extension_count, available_extensions.data());

		std::set<std::string> required_extensions(DeviceExtensionsToSupport.begin(), DeviceExtensionsToSupport.end());

		for (const VkExtensionProperties& extension : available_extensions) {
			required_extensions.erase(extension.extensionName);
		}

		if (required_extensions.empty() == false) return false;

		// 2. Check if GPU can support a swapchain
		renderer::swapchain::SwapchainOptions swapchain_support = renderer::swapchain::QuerySwapchainSupport(PhysicalDevice, CurrentSurface);
		if (swapchain_support.Formats.empty() || swapchain_support.PresentModes.empty()) return false;

		// 3. Check if GPU supports Anisotropic Filtering
		VkPhysicalDeviceFeatures supported_features{};
		vkGetPhysicalDeviceFeatures(PhysicalDevice, &supported_features);
		if (supported_features.samplerAnisotropy == false) return false;

		// 4. Check if GPU has graphics, compute and present queues
		renderer::QueueFamilyIndices queues_found = renderer::device::FindSupportedQueues(PhysicalDevice, CurrentSurface);
		if (queues_found.graphics_compute_family.has_value() == false || queues_found.present_family.has_value() == false) return false;

		// Making to end of function means all four checks passed!
		return true;
	}
}

namespace renderer::device {

	GLFWwindow* CreateVulkanWindow(const char* Title, int Width, int Height) {

		int GLFW_error_code = glfwInit();
		if (GLFW_error_code == GLFW_FALSE) {
			throw std::runtime_error("GLFW did not initialize correctly.");
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		GLFWwindow* window = glfwCreateWindow(Width, Height, Title, nullptr, nullptr);
		return window;
	}

	VkInstance CreateVulkanInstance(bool UseValidationLayers, const std::vector<const char*>& ValidationLayersToSupport, const std::vector<const char*>& InstanceExtensions) {

		if (UseValidationLayers && CheckValidationLayerSupport(ValidationLayersToSupport) == false) {
			throw std::runtime_error("Current device does not support all Validation Layers.");
		}

		VkApplicationInfo app_info = {};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pApplicationName = "Vulkan Render Test";
		app_info.applicationVersion = VK_MAKE_VERSION(1, 1, 0);
		app_info.apiVersion = VK_API_VERSION_1_1;

		VkInstanceCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		create_info.pApplicationInfo = &app_info;
		create_info.enabledExtensionCount = static_cast<uint32_t>(InstanceExtensions.size());
		create_info.ppEnabledExtensionNames = InstanceExtensions.data();

		if (UseValidationLayers) {
			create_info.enabledLayerCount = static_cast<uint32_t>(ValidationLayersToSupport.size());
			create_info.ppEnabledLayerNames = ValidationLayersToSupport.data();
		}
		else {
			create_info.enabledLayerCount = 0;
		}

		VkInstance instance = VK_NULL_HANDLE;
		if (vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS) {
			throw std::runtime_error("Vulkan failed to create instance.");
		}

		return instance;
	}

	VkSurfaceKHR CreateVulkanSurface(VkInstance VulkanInstance, GLFWwindow* Window) {

		VkSurfaceKHR vulkan_surface = VK_NULL_HANDLE;

		if (glfwCreateWindowSurface(VulkanInstance, Window, nullptr, &vulkan_surface) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create window surface!");
		}

		return vulkan_surface;
	}

	QueueFamilyIndices FindSupportedQueues(VkPhysicalDevice PhysicalDevice, VkSurfaceKHR CurrentSurface) {
		 
		QueueFamilyIndices queues_found;

		uint32_t supported_queues_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &supported_queues_count, nullptr);
		std::vector<VkQueueFamilyProperties> queues_supported_by_device(supported_queues_count);
		vkGetPhysicalDeviceQueueFamilyProperties(PhysicalDevice, &supported_queues_count, queues_supported_by_device.data());

		int index = 0;
		for (const VkQueueFamilyProperties& queue : queues_supported_by_device) {

			if ((queue.queueFlags & VK_QUEUE_GRAPHICS_BIT) && (queue.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
				queues_found.graphics_compute_family = index;
			}

			VkBool32 present_support = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(PhysicalDevice, index, CurrentSurface, &present_support);

			if (present_support) {
				queues_found.present_family = index;
			}

			if (queues_found.graphics_compute_family.has_value() && queues_found.present_family.has_value()) {
				break;
			}

			index++;
		}

		return queues_found;
	}

	VkPhysicalDevice PickPhysicalDevice(VkInstance VulkanInstance, VkSurfaceKHR VulkanSurface, const std::vector<const char*>& DeviceExtensionsToSupport) {

		VkPhysicalDevice physical_device = VK_NULL_HANDLE;

		// Check for a Vulkan supporting GPU
		uint32_t device_count = 0;
		vkEnumeratePhysicalDevices(VulkanInstance, &device_count, nullptr);
		if (device_count == 0) throw std::runtime_error("Failed to find GPUs with Vulkan support!");

		// Get all Vulkan supporting GPUS
		std::vector<VkPhysicalDevice> devices(device_count);
		vkEnumeratePhysicalDevices(VulkanInstance, &device_count, devices.data());

		// Check if a GPU supports all device extentions required.
		for (const VkPhysicalDevice& device : devices) {
			if (IsDeviceSuitable(device, VulkanSurface, DeviceExtensionsToSupport)) {
				physical_device = device;
				break;
			}
		}

		if (physical_device == VK_NULL_HANDLE) {
			throw std::runtime_error("Failed to find a suitable GPU.");
		}

		return physical_device;
	}

	VkDevice CreateLogicalDevice(const LogicalDeviceContext& Context) {

		VkPhysicalDeviceFeatures device_features{};
		device_features.samplerAnisotropy = VK_TRUE;

		std::vector<VkDeviceQueueCreateInfo> queues;
		float queue_priority = 1.0f;

		uint32_t graphics_index = Context.SupportedQueues.graphics_compute_family.value();
		uint32_t present_index = Context.SupportedQueues.present_family.value();
		std::set<uint32_t> supported_queues = { graphics_index, present_index };

		for (uint32_t queue : supported_queues) {

			VkDeviceQueueCreateInfo create_info{};
			create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			create_info.queueFamilyIndex = queue;
			create_info.queueCount = 1;
			create_info.pQueuePriorities = &queue_priority;

			queues.push_back(create_info);
		}

		VkDeviceCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		create_info.pQueueCreateInfos = queues.data();
		create_info.queueCreateInfoCount = static_cast<uint32_t>(queues.size());
		create_info.pEnabledFeatures = &device_features;
		create_info.enabledExtensionCount = static_cast<uint32_t>(Context.DeviceExtensionsToSupport.size());;
		create_info.ppEnabledExtensionNames = Context.DeviceExtensionsToSupport.data();
		create_info.enabledLayerCount = 0;

		if (Context.UseValidationLayers) {
			create_info.enabledLayerCount = static_cast<uint32_t>(Context.ValidationLayersToSupport.size());
			create_info.ppEnabledLayerNames = Context.ValidationLayersToSupport.data();
		}

		VkDevice logical_device;
		if (vkCreateDevice(Context.PhysicalDevice, &create_info, nullptr, &logical_device) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create logical device!");
		}

		return logical_device;
	}
}