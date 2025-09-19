#define SDL_MAIN_HANDLED

#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.hpp>
#include <string.h>
#include <optional>
#include <iostream>

#define WIDTH 500
#define HEIGHT 500

const std::vector<const char*> ValidationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

// Used to check Physical Devices (GPUS) for specific traits
struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;

	bool isComplete() {
		return graphicsFamily.has_value();
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

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
	QueueFamilyIndices indices;

	// Find how many queues we have in our GPU
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	// Fetch list of all Queue Families. (What type of queues and how many there are)
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	// Look for a queue family that supports graphics --

	int i = 0; // Uses seperate tracker to prevent std::optional from being set.
	for (const VkQueueFamilyProperties& queueFamily : queueFamilies) {
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}

		if (indices.isComplete()) {
			break;
		}
	}

	i++;

	// Found all requirements.
	return indices;
}

bool isDeviceSuitable(VkPhysicalDevice device) {

	// Get device information
	//VkPhysicalDeviceProperties deviceProperties;
	//VkPhysicalDeviceFeatures deviceFeatures;
	//vkGetPhysicalDeviceProperties(device, &deviceProperties);
	//vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	QueueFamilyIndices indices = findQueueFamilies(device);

	return indices.isComplete();
}

VkPhysicalDevice PickPhysicalDevice(VkInstance Instance) {

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
		if (isDeviceSuitable(device)) {
			VkDevice = device;
			break;
		}
	}

	if(VkDevice == VK_NULL_HANDLE){
		throw std::runtime_error("Failed to find a suitable GPU.");
	}

	return VkDevice;
}

VkDevice CreateLogicalDevice(VkInstance Instance, VkPhysicalDevice PhyDevice) {

	VkDevice device;

	QueueFamilyIndices indices = findQueueFamilies(PhyDevice);

	float queuePriority = 1.0f;

	// Initalize the graphics family with 1 queue
	VkDeviceQueueCreateInfo queueCreateInfo{};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.pQueuePriorities = &queuePriority;

	// List of all features we want to use
	VkPhysicalDeviceFeatures deviceFeatures{};

	// Create Logical Device
	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = &queueCreateInfo;
	createInfo.queueCreateInfoCount = 1;
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

int CreateVulkanInstance(VkInstance* VulkanInstance, SDL_Window* window){
	std::cout << "start instance " << std::endl;
	
	/// Instance selection -> A way to describe your application and any extentions you need

	VkApplicationInfo AppInfo = {};
	AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	AppInfo.pApplicationName = "Vulkan Render Test";
	AppInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	AppInfo.apiVersion = VK_API_VERSION_1_0;

	#pragma region SDL EXTENTIONS
	unsigned int SDLNumberOfExtentions;
	SDL_bool resultsGetNumber = SDL_Vulkan_GetInstanceExtensions(window, &SDLNumberOfExtentions, nullptr);
	if (resultsGetNumber == SDL_FALSE) {
		throw std::runtime_error("SDL did not fetch Number of Extentions correctly.");
	}

	// Allocate space for names of extentions
	std::vector<const char*> SDLExtentions(SDLNumberOfExtentions);

	SDL_bool resultsGetInstances = SDL_Vulkan_GetInstanceExtensions(window, &SDLNumberOfExtentions, SDLExtentions.data());
	if (resultsGetInstances == SDL_FALSE) {
		throw std::runtime_error("SDL did not fetch Instance Extentions correctly.");
	}
	#pragma endregion

	if (enableValidationLayers && !CheckValidationLayerSupport()) {
		throw std::runtime_error("Current device does not support all Validation Layers.");
	}

	VkInstanceCreateInfo InstanceCreateInfo = {};
	InstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	InstanceCreateInfo.pApplicationInfo = &AppInfo;
	InstanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(SDLNumberOfExtentions);
	InstanceCreateInfo.ppEnabledExtensionNames = SDLExtentions.data();

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

	// Init SDL
	int SDLErrorCode = SDL_Init(SDL_INIT_VIDEO);
	if (SDLErrorCode != 0) {
		throw std::runtime_error("SDL did not initialize correctly.");
	}

	// Create Window
	SDL_Window* window = SDL_CreateWindow("Vulkan Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_VULKAN);
	if (window == NULL) {
		throw std::runtime_error("SDL Window did not create correctly.");
	}

	/// Instance selection -> A way to describe your application and any extentions you need
	VkInstance VkInstance;
	int ErrorCode = CreateVulkanInstance(&VkInstance,window);
	if (ErrorCode != 0) {
		throw std::runtime_error("Instance did not create correctly.");
	}

	/// Physical device selection -> GPU selection
	VkPhysicalDevice VkPhyDevice = PickPhysicalDevice(VkInstance);

	// Logical device selection -> Specify which features and queue families
	VkDevice VkLogicDevice = CreateLogicalDevice(VkInstance, VkPhyDevice);
	
	// Get graphics queue reference.
	VkQueue graphicsQueue;
	QueueFamilyIndices indices = findQueueFamilies(VkPhyDevice);
	vkGetDeviceQueue(VkLogicDevice, indices.graphicsFamily.value(), 0, &graphicsQueue);

	// Main Application Loop
	bool appRunning = true;
	while (appRunning) {

		SDL_Event event;
		while (SDL_PollEvent(&event)) {

			switch (event.type) {

			case SDL_QUIT:
				appRunning = false;
				break;

			default:
				// Do nothing.
				break;
			}
		}

		SDL_Delay(10);
	}

	// Clean up
	vkDestroyDevice(VkLogicDevice, nullptr);
	vkDestroyInstance(VkInstance, nullptr); // Cleanup instance LAST
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}