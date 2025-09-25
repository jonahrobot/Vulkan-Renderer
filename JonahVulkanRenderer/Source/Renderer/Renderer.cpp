#include "Renderer.h"
#include "Instance.h"

namespace renderer::detail {

	// Detail namespace seperates implementation logic of utility functions into their own classes.
	// Renderer::detail naming convention tells us these are specific implementations for our renderer class.

	VkInstance CreateVulkanInstance(bool UseValidationLayers, const std::vector<const char*>& ValidationLayersToSupport);

	struct SwapChainContext{
		VkSurfaceKHR vulkan_surface;
		VkPhysicalDevice physical_device;
		VkDevice logical_device;
		GLFWwindow* window;
	};
	VkSwapchainKHR CreateSwapChain(SwapChainContext context);

	struct PhysicalDeviceContext{
		VkInstance vulkan_instance;
		VkSurfaceKHR vulkan_surface;
	};
	VkPhysicalDevice PickPhysicalDevice(PhysicalDeviceContext context);

	struct LogicalDeviceContext {
		VkInstance vulkan_instance;
		VkSurfaceKHR vulkan_surface;
		VkPhysicalDevice physical_device;
	};
	VkDevice CreateLogicalDevice(LogicalDeviceContext context);

};

namespace renderer {

	// Unnamed namespace to show functions below are pure Utility with no internal state. 
	// Cannot see or access private data of Renderer class.
	namespace{

		GLFWwindow* CreateGLFWWindow() {

			int GLFWErrorCode = glfwInit();
			if (GLFWErrorCode == GLFW_FALSE) {
				throw std::runtime_error("GLFW did not initialize correctly.");
			}

			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

			return glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
		}

		VkSurfaceKHR CreateVulkanSurface(VkInstance VulkanInstance, GLFWwindow* Window) {

			VkSurfaceKHR vulkanSurface;

			if (glfwCreateWindowSurface(VulkanInstance, Window, nullptr, &vulkanSurface) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create window surface!");
			}

			return vulkanSurface;
		}
	}

	Renderer::Renderer() {

		window = CreateGLFWWindow();
		vulkan_instance = renderer::detail::CreateVulkanInstance(UseValidationLayers, ValidationLayersToSupport);
		vulkan_surface = CreateVulkanSurface(vulkan_instance, window);

	}

	Renderer::~Renderer() {

	}

	void Renderer::Draw() {

	}

}// namespace renderer