#include "Instance.h"
#include <iostream>

namespace renderer {

VkInstance Instance::CreateInstance(const GLFWwindow* window, const std::vector<const char*>& ValidationLayers){

	VkApplicationInfo AppInfo = {};
	AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	AppInfo.pApplicationName = "Vulkan Render Test";
	AppInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	AppInfo.apiVersion = VK_API_VERSION_1_0;

	uint32_t GLFWNumberOfExtentions = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&GLFWNumberOfExtentions);

	VkInstanceCreateInfo InstanceCreateInfo = {};
	InstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	InstanceCreateInfo.pApplicationInfo = &AppInfo;
	InstanceCreateInfo.enabledExtensionCount = GLFWNumberOfExtentions;
	InstanceCreateInfo.ppEnabledExtensionNames = glfwExtensions;

	// Add all ValidationLayers if enabled
	if (enableValidationLayers) {

		if (CheckValidationLayerSupport(ValidationLayers) == false) {
			throw std::runtime_error("Current device does not support all Validation Layers.");
		}

		InstanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
		InstanceCreateInfo.ppEnabledLayerNames = ValidationLayers.data();
	}
	else {
		InstanceCreateInfo.enabledLayerCount = 0;
	}

	// Create our instance!
	VkInstance outInstance;
	VkResult out = vkCreateInstance(&InstanceCreateInfo, nullptr, &outInstance);
	if (out != VK_SUCCESS) {
		throw std::runtime_error("Vulkan failed to create instance.");
	}

	return outInstance;
}

bool Instance::CheckValidationLayerSupport(const std::vector<const char*>& ValidationLayers) {

	// Get all Supported Layers
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> supportedLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, supportedLayers.data());

	// Check if each layer in ValidationLayers is in our supported list!
	for (const char* currentLayerName : ValidationLayers) {
		bool layerFound = false;

		for (const VkLayerProperties& layerProperty : supportedLayers) {
			if (strcmp(currentLayerName, layerProperty.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}

} // namespace renderer