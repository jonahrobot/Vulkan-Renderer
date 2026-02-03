#include <iostream>
#include "RendererDetail.h"

// Unnamed namespace to show functions below are pure Utility strictly for this .cpp file.
namespace { 

bool CheckValidationLayerSupport(const std::vector<const char*>& ValidationLayersToSupport) {

	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> supportedLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, supportedLayers.data());

	for (const auto& ValidationLayerName : ValidationLayersToSupport) {
		bool layerFound = false;

		for (const auto& layerProperty : supportedLayers) {
			if (strcmp(ValidationLayerName, layerProperty.layerName) == 0) {
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

}

// Implements all Vulkan Instance creation functions in "RendererDetail.h" to be used in "Renderer.cpp"
namespace renderer::detail {

Instance_Stage CreateVulkanInstance(GLFWwindow* window, bool UseValidationLayers, const std::vector<const char*>& ValidationLayersToSupport, const std::vector<const char*>& InstanceExtensions){

	if (UseValidationLayers && CheckValidationLayerSupport(ValidationLayersToSupport) == false) {
		throw std::runtime_error("Current device does not support all Validation Layers.");	
	}

	VkApplicationInfo AppInfo = {};
	AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	AppInfo.pApplicationName = "Vulkan Render Test";  
	AppInfo.applicationVersion = VK_MAKE_VERSION(1, 1, 0);
	AppInfo.apiVersion = VK_API_VERSION_1_1;

	VkInstanceCreateInfo CreateInfo = {};
	CreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	CreateInfo.pApplicationInfo = &AppInfo;
	CreateInfo.enabledExtensionCount = static_cast<uint32_t>(InstanceExtensions.size());
	CreateInfo.ppEnabledExtensionNames = InstanceExtensions.data();

	if (UseValidationLayers) {
		CreateInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayersToSupport.size());
		CreateInfo.ppEnabledLayerNames = ValidationLayersToSupport.data();
	}
	else {
		CreateInfo.enabledLayerCount = 0;
	}

	VkInstance VulkanInstance;

	if (vkCreateInstance(&CreateInfo, nullptr, &VulkanInstance) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan failed to create instance.");
	}

	return {VulkanInstance, window};
}

} // namespace renderer::detail