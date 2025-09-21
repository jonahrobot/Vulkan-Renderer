#define SDL_MAIN_HANDLED

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

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

// Check if all layers in ValidationLayers are avaliable on our PC
bool CheckValidationLayerSupport() {

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

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
	QueueFamilyIndices indices;

	// Find how many queues we have in our GPU
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	// Fetch list of all Queue Families. (What type of queues and how many there are)
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	// Look for a queue family that supports graphics and presentation --

	int i = 0; // Uses seperate tracker to prevent std::optional from being set.
	for (const VkQueueFamilyProperties& queueFamily : queueFamilies) {
		
		// Check for graphics
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}

		// Check for present
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

		if (presentSupport) {
			indices.presentFamily = i;
		}

		// Check for all found
		if (indices.isComplete()) {
			break;
		}
	}

	i++;

	// Found all requirements.
	return indices;
}

bool checkDeviceExtentionSupport(VkPhysicalDevice device) {

	// Get devices list of supported extentions
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const VkExtensionProperties& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {

	// Get device information
	//VkPhysicalDeviceProperties deviceProperties;
	//VkPhysicalDeviceFeatures deviceFeatures;
	//vkGetPhysicalDeviceProperties(device, &deviceProperties);
	//vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	QueueFamilyIndices indices = findQueueFamilies(device, surface);

	bool extensionsSupported = checkDeviceExtentionSupport(device);

	return indices.isComplete() && extensionsSupported;
}

VkPhysicalDevice PickPhysicalDevice(VkInstance Instance, VkSurfaceKHR surface) {

	VkPhysicalDevice VkDevice = VK_NULL_HANDLE;

	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(Instance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		throw std::runtime_error("Failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(Instance, &deviceCount, devices.data());

	// Check for best device
	for (const VkPhysicalDevice& device : devices) {
		if (isDeviceSuitable(device, surface)) {
			VkDevice = device;
			break;
		}
	}

	if(VkDevice == VK_NULL_HANDLE){
		throw std::runtime_error("Failed to find a suitable GPU.");
	}

	return VkDevice;
}

VkDevice CreateLogicalDevice(VkInstance Instance, VkPhysicalDevice PhyDevice, VkSurfaceKHR surface) {

	VkDevice device;

	QueueFamilyIndices indices = findQueueFamilies(PhyDevice, surface);

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
	createInfo.enabledExtensionCount = 0;

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

int CreateVulkanInstance(VkInstance* VulkanInstance, GLFWwindow* window){
	std::cout << "start instance " << std::endl;
	
	/// Instance selection -> A way to describe your application and any extentions you need

	VkApplicationInfo AppInfo = {};
	AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	AppInfo.pApplicationName = "Vulkan Render Test";
	AppInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	AppInfo.apiVersion = VK_API_VERSION_1_0; 

	#pragma region GLFW EXTENTIONS
	uint32_t GLFWNumberOfExtentions = 0;
	const char** glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&GLFWNumberOfExtentions);
	#pragma endregion

	if (enableValidationLayers && !CheckValidationLayerSupport()) {
		throw std::runtime_error("Current device does not support all Validation Layers.");
	}

	VkInstanceCreateInfo InstanceCreateInfo = {};
	InstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	InstanceCreateInfo.pApplicationInfo = &AppInfo;
	InstanceCreateInfo.enabledExtensionCount = GLFWNumberOfExtentions;
	InstanceCreateInfo.ppEnabledExtensionNames = glfwExtensions;

	// Add all ValidationLayers if enabled
	if (enableValidationLayers) {
		InstanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
		InstanceCreateInfo.ppEnabledLayerNames = ValidationLayers.data();
	}
	else {
		InstanceCreateInfo.enabledLayerCount = 0;
	}

	// Create our instance!
	VkResult out = vkCreateInstance(&InstanceCreateInfo, nullptr, VulkanInstance);
	if (out != VK_SUCCESS) {
		throw std::runtime_error("Vulkan failed to create instance.");
	}

	return 0;
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

	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(VkPhyDevice, &deviceProperties);
	std::cout << "GPU chosen: " << deviceProperties.deviceName << "." << std::endl;

	// Logical device selection -> Specify which features and queue families
	VkDevice VkLogicDevice = CreateLogicalDevice(VkInstance, VkPhyDevice, VkSurface);
	
	// Get queue references
	QueueFamilyIndices indices = findQueueFamilies(VkPhyDevice, VkSurface);

	VkQueue graphicsQueue;
	vkGetDeviceQueue(VkLogicDevice, indices.graphicsFamily.value(), 0, &graphicsQueue);

	VkQueue presentQueue;
	vkGetDeviceQueue(VkLogicDevice, indices.presentFamily.value(), 0, &presentQueue);

	// Main Application Loop
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		
	}

	// Clean up
	vkDestroyDevice(VkLogicDevice, nullptr);
	vkDestroySurfaceKHR(VkInstance, VkSurface, nullptr);
	vkDestroyInstance(VkInstance, nullptr); // Cleanup instance LAST in Vulkan Cleanup

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}