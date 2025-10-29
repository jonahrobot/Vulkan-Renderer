#include <iostream>

#include "Renderer.h"
#include "Detail/RendererDetail.h"

namespace renderer {

	// Unnamed namespace to show functions below are pure Utility with no internal state. 
	// Cannot see or access private data of Renderer class.
	namespace {

		GLFWwindow* CreateGLFWWindow() {

			int GLFWErrorCode = glfwInit();
			if (GLFWErrorCode == GLFW_FALSE) {
				throw std::runtime_error("GLFW did not initialize correctly.");
			}

			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

			GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
			return window;
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

			VkSubpassDependency dependency{};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			VkRenderPassCreateInfo render_pass_info{};
			render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			render_pass_info.attachmentCount = 1;
			render_pass_info.pAttachments = &color_attachment;
			render_pass_info.subpassCount = 1;
			render_pass_info.pSubpasses = &subpass;
			render_pass_info.dependencyCount = 1;
			render_pass_info.pDependencies = &dependency;

			if (vkCreateRenderPass(LogicalDevice, &render_pass_info, nullptr, &render_pass) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create render pass.");
			}

			return render_pass;
		}
	}

	static void FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto renderer = reinterpret_cast<Renderer*>(glfwGetWindowUserPointer(window));
		renderer->framebuffer_resized = true;
	}

	Renderer::Renderer() {

		window = CreateGLFWWindow();
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, FramebufferResizeCallback);

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

		swapchain_creation_data = context_swapchain;

		renderer::detail::SwapchainData swapchain_info = {};
		swapchain_info = detail::CreateSwapchain(context_swapchain);
		swapchain = swapchain_info.swapchain;
		extent = swapchain_info.swapchain_extent;

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
		context_graphics_pipeline.swapchain_extent = extent;

		renderer::detail::GraphicsPipelineData pipeline_info = {};
		pipeline_info = detail::CreateGraphicsPipeline(context_graphics_pipeline);
		graphics_pipeline = pipeline_info.pipeline;
		graphics_pipeline_layout = pipeline_info.layout;

		// Create framebuffers
		detail::FrameBufferContext context_framebuffer = {};
		context_framebuffer.logical_device = logical_device;
		context_framebuffer.image_views = swapchain_image_views;
		context_framebuffer.render_pass = render_pass;
		context_framebuffer.swapchain_extent = extent;

		framebuffers = detail::CreateFramebuffers(context_framebuffer);

		// Create Command Heirarchy
		command_pool = detail::CreateCommandPool(logical_device, physical_device_data.queues_supported.graphicsFamily.value());
		command_buffers = detail::CreateCommandBuffers(MAX_FRAMES_IN_FLIGHT, logical_device, command_pool);

		// Create sync objects
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			image_available_semaphores.push_back(detail::CreateVulkanSemaphore(logical_device));
			in_flight_fences.push_back(detail::CreateVulkanFence(logical_device));
		}
		for (size_t i = 0; i < swapchain_images.size(); i++) {
			render_finished_semaphores.push_back(detail::CreateVulkanSemaphore(logical_device));
		}
	}

	Renderer::~Renderer() {

		vkDeviceWaitIdle(logical_device);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(logical_device, image_available_semaphores[i], nullptr);
			vkDestroyFence(logical_device, in_flight_fences[i], nullptr);
		}

		for (size_t i = 0; i < swapchain_images.size(); i++) {
			vkDestroySemaphore(logical_device, render_finished_semaphores[i], nullptr);
		}

		vkDestroyCommandPool(logical_device, command_pool, nullptr);

		for (auto framebuffer : framebuffers) {
			vkDestroyFramebuffer(logical_device, framebuffer, nullptr);
		}

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

		/// PREP DRAW

		// Ensure no frames are being drawn already
		vkWaitForFences(logical_device, 1, &in_flight_fences[current_frame], VK_TRUE, UINT64_MAX);

		uint32_t image_index;
		VkResult result = vkAcquireNextImageKHR(logical_device, swapchain, UINT64_MAX, image_available_semaphores[current_frame], VK_NULL_HANDLE, &image_index);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			RecreateSwapchainHelper();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("Failed to acquire swapchain image.");
		}

		// Only reset fence if we are continuing work
		vkResetFences(logical_device, 1, &in_flight_fences[current_frame]);

		vkResetCommandBuffer(command_buffers[current_frame], 0);

		/// DRAW

		detail::CommandRecordingContext command_context{};
		command_context.framebuffers = framebuffers;
		command_context.render_pass = render_pass;
		command_context.graphics_pipeline = graphics_pipeline;
		command_context.command_buffer = command_buffers[current_frame];
		command_context.image_write_index = image_index;
		command_context.swapchain_extent = extent;

		RecordCommandBuffer(command_context);

		VkSubmitInfo submit_info{};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore wait_semaphores[] = { image_available_semaphores[current_frame] }; // Command wont execute until semaphore is flagged.
		VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT }; // These stages will wait for above flag.
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = wait_semaphores;
		submit_info.pWaitDstStageMask = wait_stages;

		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &command_buffers[current_frame];

		VkSemaphore signal_semaphores[] = { render_finished_semaphores[image_index] }; // These will be flagged once command complete.
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = signal_semaphores;

		if (vkQueueSubmit(graphics_queue, 1, &submit_info, in_flight_fences[current_frame]) != VK_SUCCESS) { // Once queue submitted, can start next frame.
			throw std::runtime_error("Failed to submit draw command buffer.");
		}

		/// PRESENT

		VkPresentInfoKHR present_info{};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = signal_semaphores; // Wait for above command to complete before we render frame.

		VkSwapchainKHR all_swapchains[] = { swapchain };
		present_info.swapchainCount = 1;
		present_info.pSwapchains = all_swapchains;
		present_info.pImageIndices = &image_index;

		present_info.pResults = nullptr;

		result = vkQueuePresentKHR(present_queue, &present_info);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebuffer_resized) {
			framebuffer_resized = false;
			RecreateSwapchainHelper();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("Failed to present swapchain image.");
		}

		current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	GLFWwindow* Renderer::Get_Window() {
		return window;
	}

	void Renderer::RecreateSwapchainHelper() {
		detail::RecreateSwapchainContext swapchain_context{};
		swapchain_context.swapchain_creation_data = swapchain_creation_data;
		swapchain_context.render_pass = render_pass;
		swapchain_context.OLD_framebuffers = framebuffers;
		swapchain_context.OLD_swapchain = swapchain;
		swapchain_context.OLD_swapchain_image_views = swapchain_image_views;

		detail::RecreateSwapchainData out_data = detail::RecreateSwapchain(swapchain_context);

		swapchain = out_data.swapchain_data.swapchain;
		extent = out_data.swapchain_data.swapchain_extent;

		std::cout << "Recreated swapchain with new extent: " << extent.width << " X " << extent.height << "." << std::endl;

		framebuffers = out_data.framebuffers;
		swapchain_images = out_data.swapchain_images;
		swapchain_image_views = out_data.swapchain_image_views;
	}
}// namespace renderer