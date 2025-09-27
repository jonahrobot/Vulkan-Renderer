#include "Renderer.h"
#include "Detail/RendererDetail.h"

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
		vulkan_instance = detail::CreateVulkanInstance(UseValidationLayers, ValidationLayersToSupport);
		vulkan_surface = CreateVulkanSurface(vulkan_instance, window);
		
		renderer::detail::QueueFamilyIndices out_queues_supported;

		detail::PhysicalDeviceContext context_physical = {};
		context_physical.vulkan_instance = vulkan_instance;
		context_physical.vulkan_surface = vulkan_surface;
		physical_device = detail::PickPhysicalDevice(out_queues_supported, context_physical, DeviceExtensionsToSupport);

		detail::LogicalDeviceContext context_logical = {};
		context_logical.vulkan_instance = vulkan_instance;
		context_logical.vulkan_surface = vulkan_surface;
		context_logical.physical_device = physical_device;
		context_logical.supported_queues = out_queues_supported;
		context_logical.UseValidationLayers = UseValidationLayers;
		logical_device = detail::CreateLogicalDevice(context_logical, DeviceExtensionsToSupport, ValidationLayersToSupport);

		detail::SwapChainContext context_swapchain = {};
		context_swapchain.physical_device = physical_device;
		context_swapchain.vulkan_surface = vulkan_surface;
		context_swapchain.logical_device = logical_device;
		context_swapchain.window = window;
		swap_chain = detail::CreateSwapChain(context_swapchain);
	}

	Renderer::~Renderer() {

	}

	void Renderer::Draw() {

	}

}// namespace renderer