#define VK_USE_PLATFORM_WIN32_KHR

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>
#include <string.h>
#include <set>
#include <optional>
#include <limits>
#include <algorithm>
#include <iostream>

#define WIDTH 500
#define HEIGHT 500

//https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Logical_device_and_queues

const std::vector<const char*> ValidationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

// Used to check Physical Devices (GPUS) for specific traits
struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

// Used to check GPU SwapChain support hits our target traits
struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

VkDevice CreateLogicalDevice(VkInstance Instance, VkPhysicalDevice PhyDevice, VkSurfaceKHR surface) {

	VkDevice device;

	QueueFamilyIndices indices = FindSupportedQueues(PhyDevice, surface);

	// Prep for queue creation 
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	// Initalize each queue
	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		queueCreateInfos.push_back(queueCreateInfo);
	}

	// List of all features we want to use
	VkPhysicalDeviceFeatures deviceFeatures{};

	// Create Logical Device
	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());;
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	// Support device specific validation layers
	// New versions of vulkan instance validation layers also handle device validations
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
		createInfo.ppEnabledLayerNames = ValidationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(PhyDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create logical device!");
	}

	return device;
}

// Preferred: SRGB 8 bit
VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats){
	
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){
			return availableFormat;
		}
	}

	return availableFormats[0];
}

// Preferred: Mailbox present mode
VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

// Sets swapchain size to window size
VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window) {
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		// Usual case: Swapchain size == window resolution
		return capabilities.currentExtent;
	}
	else {
		// Window manager wants us to pick scale of swapchain.
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

VkSurfaceFormatKHR surfaceFormat;
VkExtent2D extent;

VkSwapchainKHR createSwapChain(VkPhysicalDevice device, VkSurfaceKHR surface, GLFWwindow* window, VkDevice logicDevice) {

	SwapChainSupportDetails swapChainSupport = GetSwapChainDetails(device, surface);

	surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	extent = chooseSwapExtent(swapChainSupport.capabilities, window);
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

	// maxImageCount of 0 means no max image count.
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	// Define Swap Chain Creation Struct
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1; // Always 1 unless stereoscopic 3D.
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = FindSupportedQueues(device, surface);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	if (indices.graphicsFamily != indices.presentFamily) {
		// Concurrent handling of queues when graphics and present logic
		// on different queues.
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		// Most common mode, graphic and present on same queue family.
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	// Modify if we want to apply transformations to images on swapchain
	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE; // Don't render pixels obsured by another window.
	createInfo.oldSwapchain = VK_NULL_HANDLE; // Must use when swapchain becomes invalid (window resize)

	VkSwapchainKHR swapChain;

	if (vkCreateSwapchainKHR(logicDevice, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create swap chain!");
	}

	return swapChain;
}

int main() {

	// Init GLFW
	int GLFWErrorCode = glfwInit();
	if (GLFWErrorCode == GLFW_FALSE) {
		throw std::runtime_error("GLFW did not initialize correctly.");
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

	if (window == NULL) {
		throw std::runtime_error("GLFW Window did not create correctly.");
	}

	/// Instance selection -> A way to describe your application and any extentions you need
	VkInstance VkInstance;
	int ErrorCode = CreateVulkanInstance(&VkInstance, window);
	if (ErrorCode != 0) {
		throw std::runtime_error("Instance did not create correctly.");
	}

	// Create Surface
	VkSurfaceKHR VkSurface;
	if (glfwCreateWindowSurface(VkInstance, window, nullptr, &VkSurface) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create window surface!");
	}

	/// Physical device selection -> GPU selection
	VkPhysicalDevice VkPhyDevice = PickPhysicalDevice(VkInstance, VkSurface);





	// Logical device selection -> Specify which features and queue families
	VkDevice VkLogicDevice = CreateLogicalDevice(VkInstance, VkPhyDevice, VkSurface);
	
	// Create SwapChain
	VkSwapchainKHR VkSwapChain = createSwapChain(VkPhyDevice, VkSurface, window, VkLogicDevice);

	/// --- ABOVE HAS BEEN MOVED, BELOW HAS NOT ---

	// Get Images
	std::vector<VkImage> swapChainImages;
	uint32_t imageCount = 0;
	vkGetSwapchainImagesKHR(VkLogicDevice, VkSwapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(VkLogicDevice, VkSwapChain, &imageCount, swapChainImages.data());

	// Get queue references
	QueueFamilyIndices indices = FindSupportedQueues(VkPhyDevice, VkSurface);

	VkQueue graphicsQueue;
	vkGetDeviceQueue(VkLogicDevice, indices.graphicsFamily.value(), 0, &graphicsQueue);

	VkQueue presentQueue;
	vkGetDeviceQueue(VkLogicDevice, indices.presentFamily.value(), 0, &presentQueue);

	// Main Application Loop
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();	
	}

	// Clean up
	vkDestroySwapchainKHR(VkLogicDevice, VkSwapChain, nullptr);
	vkDestroyDevice(VkLogicDevice, nullptr);
	vkDestroySurfaceKHR(VkInstance, VkSurface, nullptr);
	vkDestroyInstance(VkInstance, nullptr); // Cleanup instance LAST in Vulkan Cleanup

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}