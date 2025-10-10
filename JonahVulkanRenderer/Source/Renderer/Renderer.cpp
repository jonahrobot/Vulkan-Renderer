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

		VkRenderPass CreateRenderPass(const VkDevice LogicalDevice, const VkFormat SwapChainFormat) {

			VkRenderPass render_pass;

			VkAttachmentDescription color_attachment{};
			color_attachment.format = SwapChainFormat;
			color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
			color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			VkAttachmentReference color_attachment_reference{};
			color_attachment_reference.attachment = 0;
			color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &color_attachment_reference;

			VkRenderPassCreateInfo render_pass_info{};
			render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			render_pass_info.attachmentCount = 1;
			render_pass_info.pAttachments = &color_attachment;
			render_pass_info.subpassCount = 1;
			render_pass_info.pSubpasses = &subpass;

			if (vkCreateRenderPass(LogicalDevice, &render_pass_info, nullptr, &render_pass) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create render pass.");
			}

			return render_pass;
		}

		struct FrameBufferContext {
			std::vector<VkImageView> ImageViews;
			VkRenderPass RenderPass;
			VkExtent2D SwapChainExtent;
		};
		std::vector<VkFramebuffer> CreateFramebuffers(const FrameBufferContext& context) {

		}
	}

	Renderer::Renderer() {

		window = CreateGLFWWindow();
		vulkan_instance = detail::CreateVulkanInstance(UseValidationLayers, ValidationLayersToSupport);
		vulkan_surface = CreateVulkanSurface(vulkan_instance, window);

		// Create Physical Device
		detail::PhysicalDeviceContext context_physical = {};
		context_physical.vulkan_instance = vulkan_instance;
		context_physical.vulkan_surface = vulkan_surface;
		context_physical.DeviceExtensionsToSupport = DeviceExtensionsToSupport;

		renderer::detail::PhysicalDeviceData physical_device_data = {};
		physical_device_data = detail::PickPhysicalDevice(context_physical);
		physical_device = physical_device_data.physical_device;

		// Create Logical Device
		detail::LogicalDeviceContext context_logical = {};
		context_logical.vulkan_instance = vulkan_instance;
		context_logical.vulkan_surface = vulkan_surface;
		context_logical.physical_device = physical_device;
		context_logical.supported_queues = physical_device_data.queues_supported;
		context_logical.UseValidationLayers = UseValidationLayers;
		context_logical.DeviceExtensionsToSupport = DeviceExtensionsToSupport;
		context_logical.ValidationLayersToSupport = ValidationLayersToSupport;

		logical_device = detail::CreateLogicalDevice(context_logical);

		// Create Swapchain
		detail::SwapchainContext context_swapchain = {};
		context_swapchain.physical_device = physical_device;
		context_swapchain.vulkan_surface = vulkan_surface;
		context_swapchain.logical_device = logical_device;
		context_swapchain.window = window;
		context_swapchain.supported_queues = physical_device_data.queues_supported;
		context_swapchain.swapchain_support_details = physical_device_data.swapchain_support_details;

		renderer::detail::SwapchainData swapchain_info = {};
		swapchain_info = detail::CreateSwapchain(context_swapchain);
		swapchain = swapchain_info.swapchain;

		// Create images
		swapchain_images = detail::GetSwapchainImages(swapchain, logical_device);
		swapchain_image_views = detail::CreateSwapchainViews(swapchain_images, logical_device, swapchain_info.swapchain_image_format);

		// Create queues
		vkGetDeviceQueue(logical_device, physical_device_data.queues_supported.graphicsFamily.value(), 0, &graphics_queue);
		vkGetDeviceQueue(logical_device, physical_device_data.queues_supported.presentFamily.value(), 0, &present_queue);
	
		// Create render pass
		render_pass = CreateRenderPass(logical_device, swapchain_info.swapchain_image_format);

		// Create graphics pipeline
		detail::GraphicsPipelineContext context_graphics_pipeline = {};
		context_graphics_pipeline.logical_device = logical_device;
		context_graphics_pipeline.render_pass = render_pass;
		context_graphics_pipeline.swapchain_extent = swapchain_info.swapchain_extent;

		renderer::detail::GraphicsPipelineData pipeline_info = {};
		pipeline_info = detail::CreateGraphicsPipeline(context_graphics_pipeline);
		graphics_pipeline = pipeline_info.pipeline;
		graphics_pipeline_layout = pipeline_info.layout;

	}

	Renderer::~Renderer() {

		vkDestroyPipeline(logical_device, graphics_pipeline, nullptr);
		vkDestroyPipelineLayout(logical_device, graphics_pipeline_layout, nullptr);
		vkDestroyRenderPass(logical_device, render_pass, nullptr);

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