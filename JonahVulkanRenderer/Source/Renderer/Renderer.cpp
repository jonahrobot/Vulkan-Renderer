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

		VkSurfaceKHR CreateVulkanSurface(const VkInstance VulkanInstance, GLFWwindow* Window) {

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
		
		renderer::detail::OutParams_PhysicalDevice device_support_data = {};

		detail::PhysicalDeviceContext context_physical = {};
		context_physical.vulkan_instance = vulkan_instance;
		context_physical.vulkan_surface = vulkan_surface;

		physical_device = detail::PickPhysicalDevice(device_support_data, context_physical, DeviceExtensionsToSupport);

		detail::LogicalDeviceContext context_logical = {};
		context_logical.vulkan_instance = vulkan_instance;
		context_logical.vulkan_surface = vulkan_surface;
		context_logical.physical_device = physical_device;
		context_logical.supported_queues = device_support_data.out_queues_supported;
		context_logical.UseValidationLayers = UseValidationLayers;

		logical_device = detail::CreateLogicalDevice(context_logical, DeviceExtensionsToSupport, ValidationLayersToSupport);

		renderer::detail::OutParams_Swapchain swapchain_info = {};

		detail::SwapchainContext context_swapchain = {};
		context_swapchain.physical_device = physical_device;
		context_swapchain.vulkan_surface = vulkan_surface;
		context_swapchain.logical_device = logical_device;
		context_swapchain.window = window;
		context_swapchain.supported_queues = device_support_data.out_queues_supported;
		context_swapchain.swapchain_support_details = device_support_data.out_swapchain_support_details;

		swapchain = detail::CreateSwapchain(swapchain_info, context_swapchain);

		detail::GetSwapchainImages(swapchain_images, swapchain, logical_device);
		detail::CreateSwapchainViews(swapchain_image_views, swapchain_images, logical_device, swapchain_info.swapchain_image_format, swapchain_info.swapchain_extent);

		vkGetDeviceQueue(logical_device, device_support_data.out_queues_supported.graphicsFamily.value(), 0, &graphics_queue);
		vkGetDeviceQueue(logical_device, device_support_data.out_queues_supported.presentFamily.value(), 0, &present_queue);
	}

	Renderer::~Renderer() {

		for (auto view : swapchain_image_views) {
			vkDestroyImageView(logical_device, view, nullptr);
		}

		vkDestroySwapchainKHR(logical_device, swapchain, nullptr);
		vkDestroyDevice(logical_device, nullptr);
		vkDestroySurfaceKHR(vulkan_instance, vulkan_surface, nullptr);
		vkDestroyInstance(vulkan_instance, nullptr); // Cleanup instance LAST in Vulkan Cleanup

		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void Renderer::Draw() {

	}

	GLFWwindow* Renderer::Get_Window() {
		return window;
	}

}// namespace renderer