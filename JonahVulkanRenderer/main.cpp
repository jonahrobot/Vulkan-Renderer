#define SDL_MAIN_HANDLED

#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.hpp>
#include <string.h>
#include <iostream>

#define WIDTH 500
#define HEIGHT 500

const std::vector<const char*> ValidationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

int ThrowError(std::string errorMessage) {
	std::cout << errorMessage << std::endl;
	SDL_Delay(2000);
	return 1;
}

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
		return ThrowError("SDL did not fetch Number of Extentions correctly.");
	}

	// Allocate space for names of extentions
	std::vector<const char*> SDLExtentions(SDLNumberOfExtentions);

	SDL_bool resultsGetInstances = SDL_Vulkan_GetInstanceExtensions(window, &SDLNumberOfExtentions, SDLExtentions.data());
	if (resultsGetInstances == SDL_FALSE) {
		return ThrowError("SDL did not fetch Instance Extentions correctly.");
	}
	#pragma endregion

	if (enableValidationLayers && !CheckValidationLayerSupport()) {
		return ThrowError("Current device does not support all Validation Layers.");
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
		return ThrowError("Vulkan failed to create instance.");
	}

	return 0;
}

int main() {

	// Init SDL
	int SDLErrorCode = SDL_Init(SDL_INIT_VIDEO);
	if (SDLErrorCode != 0) {
		return ThrowError("SDL did not initialize correctly.");
	}

	// Create Window
	SDL_Window* window = SDL_CreateWindow("Vulkan Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_VULKAN);
	if (window == NULL) {
		return ThrowError("SDL Window did not create correctly.");
	}

	/// Instance selection -> A way to describe your application and any extentions you need

	VkInstance VkInstance;
	int ErrorCode = CreateVulkanInstance(&VkInstance,window);
	if (ErrorCode != 0) {
		return ThrowError("Instance did not create correctly.");
	}

	/// Physical device selection -> GPU selection


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
	vkDestroyInstance(VkInstance, nullptr); // Cleanup instance LAST
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}