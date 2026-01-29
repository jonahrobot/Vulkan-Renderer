
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#include <unordered_map>
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

		VkRenderPass CreateRenderPass(const VkDevice LogicalDevice, const VkFormat SwapChainFormat, const VkPhysicalDevice PhysicalDevice) {

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

			VkAttachmentDescription depth_attachment{};
			depth_attachment.format = renderer::detail::FindDepthFormat(PhysicalDevice);
			depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
			depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkAttachmentReference depth_attachment_reference{};
			depth_attachment_reference.attachment = 1;
			depth_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subpass{};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &color_attachment_reference;
			subpass.pDepthStencilAttachment = &depth_attachment_reference;

			VkSubpassDependency dependency{};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			std::array<VkAttachmentDescription, 2> attachments = { color_attachment, depth_attachment };

			VkRenderPassCreateInfo render_pass_info{};
			render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			render_pass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
			render_pass_info.pAttachments = attachments.data();
			render_pass_info.subpassCount = 1;
			render_pass_info.pSubpasses = &subpass;
			render_pass_info.dependencyCount = 1;
			render_pass_info.pDependencies = &dependency;

			if (vkCreateRenderPass(LogicalDevice, &render_pass_info, nullptr, &render_pass) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create render pass.");
			}

			return render_pass;
		}

		std::vector<detail::InstanceData> ProcessInstanceData(const std::vector<detail::InstanceModelData>& NewModelSet) {

			std::vector<detail::InstanceData> instance_data = {};

			for (detail::InstanceModelData model_data : NewModelSet) {

				for (uint32_t i = 0; i < model_data.instance_count; i++) {

					detail::InstanceData new_instance;

					new_instance.model = model_data.instance_model_matrices[i];
					new_instance.array_index.x = 0;

					instance_data.push_back(new_instance);
				}
			}

			return instance_data;
		}

		detail::UniformBufferObject GetNextUBO(VkExtent2D Extent, glm::mat4 CameraPosition) {

			detail::UniformBufferObject ubo = {};

			ubo.proj = glm::perspective(glm::radians(45.0f), Extent.width / (float)Extent.height, 0.01f, 100.0f);
			ubo.proj[1][1] *= -1;
			ubo.view = CameraPosition;

			glm::mat4 matrix = ubo.proj * ubo.view;
			matrix = glm::transpose(matrix);

			enum side { LEFT = 0, RIGHT = 1, TOP = 2, BOTTOM = 3, NEAR_ = 4, FAR_ = 5 };

			ubo.frustum_planes[LEFT] = matrix[3] + matrix[0];
			ubo.frustum_planes[RIGHT] = matrix[3] - matrix[0];
			ubo.frustum_planes[TOP] = matrix[3] - matrix[1];
			ubo.frustum_planes[BOTTOM] = matrix[3] + matrix[1];
			ubo.frustum_planes[NEAR_] = matrix[2];
			ubo.frustum_planes[FAR_] = matrix[3] - matrix[2];

			for (auto i = 0; i < 6; i++)
			{
				float length = glm::length(glm::vec3(ubo.frustum_planes[i]));
				ubo.frustum_planes[i] /= length;
			}

			return ubo;
		}

		VkSampler CreateTextureSampler(const VkPhysicalDevice& PhysicalDevice, const VkDevice& LogicalDevice) {

			VkSampler our_sampler;

			VkSamplerCreateInfo sampler_info{};
			sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			sampler_info.magFilter = VK_FILTER_LINEAR;
			sampler_info.minFilter = VK_FILTER_LINEAR;

			sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

			sampler_info.anisotropyEnable = VK_TRUE;

			VkPhysicalDeviceProperties properties{};
			vkGetPhysicalDeviceProperties(PhysicalDevice, &properties);

			sampler_info.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
			sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
			sampler_info.unnormalizedCoordinates = VK_FALSE;
			sampler_info.compareEnable = VK_FALSE;
			sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
			sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			sampler_info.mipLodBias = 0.0f;
			sampler_info.minLod = 0.0f;
			sampler_info.maxLod = 0.0f;

			if (vkCreateSampler(LogicalDevice, &sampler_info, nullptr, &our_sampler) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create texture sample.");
			}

			return our_sampler;
		}

		std::vector<VkDrawIndexedIndirectCommand> RecordIndirectCommands(std::vector<detail::Vertex>& VerticeToRender, std::vector<uint32_t>& Indices, uint32_t& NumberOfMeshes, const std::vector<detail::InstanceModelData>& NewModelSet) {
			std::vector<VkDrawIndexedIndirectCommand> indirect_commands;

			uint32_t m = 0;
			NumberOfMeshes = 0;
			VerticeToRender.clear();
			Indices.clear();

			for (detail::InstanceModelData model_usage_data : NewModelSet) {

				detail::ModelData model = model_usage_data.model_data;

				bool no_data = model.vertices.size() == 0 || model.indices.size() == 0;
				if (no_data) continue;

				NumberOfMeshes += model_usage_data.instance_count;

				uint32_t offset = static_cast<uint32_t>(VerticeToRender.size());

				for (detail::Vertex v : model.vertices) {
					VerticeToRender.push_back(v);
				}

				uint32_t first_index = static_cast<uint32_t>(Indices.size());

				for (uint32_t i : model.indices) {
					Indices.push_back(i + offset);
				}

				// Create draw command
				VkDrawIndexedIndirectCommand indirect_command{};
				indirect_command.instanceCount = model_usage_data.instance_count;
				indirect_command.firstInstance = m;
				indirect_command.firstIndex = first_index;
				indirect_command.indexCount = static_cast<uint32_t>(model.indices.size());

				indirect_commands.push_back(indirect_command);

				m += model_usage_data.instance_count;
			}

			return indirect_commands;
		}

	} // namespace util

	static void FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto renderer = reinterpret_cast<Renderer*>(glfwGetWindowUserPointer(window));
		renderer->framebuffer_resized = true;
	}

	Renderer::Renderer() {

		window = CreateGLFWWindow();
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, FramebufferResizeCallback);

		uint32_t number_of_extentions = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&number_of_extentions);

		for (int i = 0; i < number_of_extentions; i++) {
			InstanceExtensionsToSupport.push_back(glfwExtensions[i]);
		}

		vulkan_instance = detail::CreateVulkanInstance(UseValidationLayers, ValidationLayersToSupport, InstanceExtensionsToSupport);
		vulkan_surface = CreateVulkanSurface(vulkan_instance, window);

		// Fetch functions
		pfn_CmdBeginDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(vkGetInstanceProcAddr(vulkan_instance, "vkCmdBeginDebugUtilsLabelEXT"));
		pfn_CmdEndDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(vkGetInstanceProcAddr(vulkan_instance, "vkCmdEndDebugUtilsLabelEXT"));

		// Create Physical Device
		detail::PhysicalDeviceContext context_physical = {};
		context_physical.vulkan_instance = vulkan_instance;
		context_physical.vulkan_surface = vulkan_surface;
		context_physical.DeviceExtensionsToSupport = DeviceExtensionsToSupport;

		detail::PhysicalDeviceData physical_device_data = detail::PickPhysicalDevice(context_physical);
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

		detail::SwapchainData swapchain_info = detail::CreateSwapchain(context_swapchain);
		swapchain = swapchain_info.swapchain;
		extent = swapchain_info.swapchain_extent;

		// Create images
		swapchain_images = detail::GetSwapchainImages(swapchain, logical_device);
		swapchain_image_views = detail::CreateSwapchainViews(swapchain_images, logical_device, swapchain_info.swapchain_image_format);

		// Create queues
		vkGetDeviceQueue(logical_device, physical_device_data.queues_supported.graphics_compute_family.value(), 0, &graphics_queue);
		vkGetDeviceQueue(logical_device, physical_device_data.queues_supported.present_family.value(), 0, &present_queue);

		// Currently not using Async compute, in future can swap compute_queue to not be equal to graphics_queue
		vkGetDeviceQueue(logical_device, physical_device_data.queues_supported.graphics_compute_family.value(), 0, &compute_queue);

		// Create render pass
		render_pass = CreateRenderPass(logical_device, swapchain_info.swapchain_image_format, physical_device);

		// Create descriptor set for graphics and compute
		descriptor_set_layout = detail::CreateDescriptorLayout(logical_device);

		// Create graphics pipeline
		detail::GraphicsPipelineContext context_graphics_pipeline = {};
		context_graphics_pipeline.logical_device = logical_device;
		context_graphics_pipeline.render_pass = render_pass;
		context_graphics_pipeline.swapchain_extent = extent;
		context_graphics_pipeline.descriptor_set_layout = descriptor_set_layout;

		detail::PipelineData pipeline_info = detail::CreateGraphicsPipeline(context_graphics_pipeline);
		graphics_pipeline = pipeline_info.pipeline;
		graphics_pipeline_layout = pipeline_info.layout;

		// Create compute pipeline
		detail::ComputePipelineContext context_compute_pipeline = {};
		context_compute_pipeline.logical_device = logical_device;
		context_compute_pipeline.descriptor_set_layout = descriptor_set_layout;

		detail::PipelineData compute_pipeline_info = detail::CreateComputePipeline(context_compute_pipeline);
		compute_pipeline = compute_pipeline_info.pipeline;
		compute_pipeline_layout = compute_pipeline_info.layout;

		// Create depth buffer
		detail::DepthBufferContext context_depth_buffer = {};
		context_depth_buffer.logical_device = logical_device;
		context_depth_buffer.physical_device = physical_device;
		context_depth_buffer.swapchain_extent = extent;

		depth_buffer = detail::CreateDepthBuffer(context_depth_buffer);

		// Create framebuffers
		detail::FrameBufferContext context_framebuffer = {};
		context_framebuffer.logical_device = logical_device;
		context_framebuffer.image_views = swapchain_image_views;
		context_framebuffer.render_pass = render_pass;
		context_framebuffer.swapchain_extent = extent;
		context_framebuffer.depth_image_view = depth_buffer.image_view;

		framebuffers = detail::CreateFramebuffers(context_framebuffer);

		// Create Command Heirarchy
		command_pool = detail::CreateCommandPool(logical_device, physical_device_data.queues_supported.graphics_compute_family.value());
		command_buffers = detail::CreateCommandBuffers(MAX_FRAMES_IN_FLIGHT, logical_device, command_pool);

		compute_command_pool = detail::CreateCommandPool(logical_device, physical_device_data.queues_supported.graphics_compute_family.value());
		compute_command_buffers = detail::CreateCommandBuffers(MAX_FRAMES_IN_FLIGHT, logical_device, compute_command_pool);

		// Create Texture Sampler
		texture_sampler = CreateTextureSampler(physical_device, logical_device);

		// Create UBO for Vertex
		detail::UniformBufferContext context_ubo = {};
		context_ubo.logical_device = logical_device;
		context_ubo.max_frames_in_flight = MAX_FRAMES_IN_FLIGHT;
		context_ubo.physical_device = physical_device;
		context_ubo.ubo_size = sizeof(detail::UniformBufferObject);

		detail::UniformBufferData ubo_info = detail::CreateUniformBuffers(context_ubo);
		uniform_buffers = ubo_info.uniform_buffers;
		uniform_buffers_memory = ubo_info.uniform_buffers_memory;
		uniform_buffers_mapped = ubo_info.uniform_buffers_mapped;

		// Create Descriptor Pool
		descriptor_pool = detail::CreateDescriptorPool(logical_device, MAX_FRAMES_IN_FLIGHT);

		// Create Descriptor Sets
		detail::DescriptorCreateContext context_descriptor_set = {};
		context_descriptor_set.descriptor_pool = descriptor_pool;
		context_descriptor_set.descriptor_set_layout = descriptor_set_layout;
		context_descriptor_set.logical_device = logical_device;
		context_descriptor_set.max_frames_in_flight = MAX_FRAMES_IN_FLIGHT;

		descriptor_sets = detail::CreateDescriptorSets(context_descriptor_set);

		// Create sync objects
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {

			// Graphics
			image_available_semaphores.push_back(detail::CreateVulkanSemaphore(logical_device));
			in_flight_fences.push_back(detail::CreateVulkanFence(logical_device));

			// Compute
			compute_in_flight_fences.push_back(detail::CreateVulkanFence(logical_device));
			compute_finished_semaphores.push_back(detail::CreateVulkanSemaphore(logical_device));
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
			vkDestroySemaphore(logical_device, compute_finished_semaphores[i], nullptr);
			vkDestroyFence(logical_device, compute_in_flight_fences[i], nullptr);
		}

		for (size_t i = 0; i < swapchain_images.size(); i++) {
			vkDestroySemaphore(logical_device, render_finished_semaphores[i], nullptr);
		}

		vkDestroySampler(logical_device, texture_sampler, nullptr);

		detail::FreeGPUResource(texture_buffer, logical_device);
		detail::FreeGPUResource(depth_buffer, logical_device);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroyBuffer(logical_device, uniform_buffers[i], nullptr);
			vkFreeMemory(logical_device, uniform_buffers_memory[i], nullptr);
		}

		vkDestroyBuffer(logical_device, instance_centers_buffer, nullptr);
		vkFreeMemory(logical_device, instance_centers_buffer_memory, nullptr);

		vkDestroyDescriptorPool(logical_device, descriptor_pool, nullptr);

		vkDestroyDescriptorSetLayout(logical_device, descriptor_set_layout, nullptr); 

		vkDestroyCommandPool(logical_device, command_pool, nullptr);

		vkDestroyCommandPool(logical_device, compute_command_pool, nullptr);

		for (auto framebuffer : framebuffers) {
			vkDestroyFramebuffer(logical_device, framebuffer, nullptr);
		}

		vkDestroyPipeline(logical_device, graphics_pipeline, nullptr);
		vkDestroyPipelineLayout(logical_device, graphics_pipeline_layout, nullptr);
		vkDestroyPipeline(logical_device, compute_pipeline, nullptr);
		vkDestroyPipelineLayout(logical_device, compute_pipeline_layout, nullptr);
		vkDestroyRenderPass(logical_device, render_pass, nullptr);

		for (auto view : swapchain_image_views) {
			vkDestroyImageView(logical_device, view, nullptr);
		}

		vkDestroySwapchainKHR(logical_device, swapchain, nullptr);

		vkDestroyBuffer(logical_device, index_buffer, nullptr);
		vkFreeMemory(logical_device, index_buffer_memory, nullptr);

		vkDestroyBuffer(logical_device, vertex_buffer, nullptr);
		vkFreeMemory(logical_device, vertex_buffer_memory, nullptr);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroyBuffer(logical_device, indirect_command_buffers[i], nullptr);
			vkFreeMemory(logical_device, indirect_command_buffer_memorys[i], nullptr);
		}

		vkDestroyBuffer(logical_device, instance_data_buffer, nullptr);
		vkFreeMemory(logical_device, instance_data_buffer_memory, nullptr);

		vkDestroyDevice(logical_device, nullptr);
		vkDestroySurfaceKHR(vulkan_instance, vulkan_surface, nullptr);
		vkDestroyInstance(vulkan_instance, nullptr); // Cleanup instance LAST in Vulkan Cleanup

		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void Renderer::Draw(glm::mat4 CameraPosition) {

		/// PREP DRAW

		// COMPUTE WORKLOAD
		vkWaitForFences(logical_device, 1, &in_flight_fences[current_frame], VK_TRUE, UINT64_MAX);
		vkWaitForFences(logical_device, 1, &compute_in_flight_fences[current_frame], VK_TRUE, UINT64_MAX);

		// Get Camera position
		detail::UniformBufferObject current_ubo_data = GetNextUBO(extent, CameraPosition);

		memcpy(uniform_buffers_mapped[current_frame], &current_ubo_data, sizeof(detail::UniformBufferObject));

		vkResetFences(logical_device, 1, &compute_in_flight_fences[current_frame]);

		vkResetCommandBuffer(compute_command_buffers[current_frame], 0);

		detail::Compute_CommandRecordingContext compute_command_context{};
		compute_command_context.command_buffer = compute_command_buffers[current_frame];
		compute_command_context.compute_pipeline = compute_pipeline;
		compute_command_context.compute_pipeline_layout = compute_pipeline_layout;
		compute_command_context.current_descriptor_set = descriptor_sets[current_frame];
		compute_command_context.instance_count = object_count;
		compute_command_context.debug_function_begin = pfn_CmdBeginDebugUtilsLabelEXT;
		compute_command_context.debug_function_end = pfn_CmdEndDebugUtilsLabelEXT;

		RecordCommandBuffer(compute_command_context);
		
		VkSubmitInfo submit_info_compute{};
		submit_info_compute.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info_compute.commandBufferCount = 1;
		submit_info_compute.pCommandBuffers = &compute_command_buffers[current_frame];
		submit_info_compute.signalSemaphoreCount = 1;
		submit_info_compute.pSignalSemaphores = &compute_finished_semaphores[current_frame];

		if (vkQueueSubmit(compute_queue, 1, &submit_info_compute, compute_in_flight_fences[current_frame]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to submit compute command buffer.");
		}

		// GRAPHICS WORKLOAD

		uint32_t image_index;
		VkResult result = vkAcquireNextImageKHR(logical_device, swapchain, UINT64_MAX, image_available_semaphores[current_frame], VK_NULL_HANDLE, &image_index);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			RecreateSwapchainHelper();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("Failed to acquire swapchain image.");
		} 

		vkResetFences(logical_device, 1, &in_flight_fences[current_frame]);

		vkResetCommandBuffer(command_buffers[current_frame], 0);
		
		/// DRAW
		detail::CommandRecordingContext command_context{};
		command_context.framebuffers = framebuffers;
		command_context.render_pass = render_pass;
		command_context.graphics_pipeline = graphics_pipeline;
		command_context.graphics_pipeline_layout = graphics_pipeline_layout;
		command_context.command_buffer = command_buffers[current_frame];
		command_context.current_descriptor_set = descriptor_sets[current_frame];
		command_context.image_write_index = image_index;
		command_context.swapchain_extent = extent;
		command_context.vertex_buffer = vertex_buffer;
		command_context.total_vertices = static_cast<uint32_t>(vertices_to_render.size());
		command_context.index_buffer = index_buffer;
		command_context.total_indices = static_cast<uint32_t>(indices.size());
		command_context.indirect_command_buffer = indirect_command_buffers[current_frame];
		command_context.number_of_draw_calls = number_of_indirect_commands;
		command_context.debug_function_begin = pfn_CmdBeginDebugUtilsLabelEXT;
		command_context.debug_function_end = pfn_CmdEndDebugUtilsLabelEXT;

		RecordCommandBuffer(command_context);

		VkSubmitInfo submit_info{};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore wait_semaphores[] = { compute_finished_semaphores[current_frame], image_available_semaphores[current_frame] }; // Command wont execute until semaphores are flagged.
		VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT , VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT }; // These stages will wait for above flag.
		submit_info.waitSemaphoreCount = 2;
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


	/// TODO: UPDATE this function to support ModelWithUsage Data
	// Will still merge index and vertex data, but now will be more specific with instanced data with specific model matrices and such!
	// This will be changes in this function, RecordIndirectCommands and ProcessInstanceData
	void Renderer::UpdateModelSet(std::vector<detail::InstanceModelData> NewModelSet, bool UseWhiteTexture) {

		/*for (detail::ModelData model : NewModelSet) {
			if (detail::VerifyModel(model) != true) {
				return;
			}
		}*/

		vkDeviceWaitIdle(logical_device);

		std::vector<VkDrawIndexedIndirectCommand> indirect_commands = RecordIndirectCommands(vertices_to_render, indices, object_count, NewModelSet);
		number_of_indirect_commands = static_cast<uint32_t>(indirect_commands.size());

		if (index_buffer != NULL) {
			vkDestroyBuffer(logical_device, index_buffer, nullptr);
			vkFreeMemory(logical_device, index_buffer_memory, nullptr);
		}

		if (vertex_buffer != NULL) {
			vkDestroyBuffer(logical_device, vertex_buffer, nullptr);
			vkFreeMemory(logical_device, vertex_buffer_memory, nullptr);
		}

		if (instance_centers_buffer != NULL) {
			vkDestroyBuffer(logical_device, instance_centers_buffer, nullptr);
			vkFreeMemory(logical_device, instance_centers_buffer_memory, nullptr);
		}
	
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			if (indirect_command_buffers[i] != NULL) {
				vkDestroyBuffer(logical_device, indirect_command_buffers[i], nullptr);
				vkFreeMemory(logical_device, indirect_command_buffer_memorys[i], nullptr);
			}
		}

		if (instance_data_buffer != NULL) {
			vkDestroyBuffer(logical_device, instance_data_buffer, nullptr);
			vkFreeMemory(logical_device, instance_data_buffer_memory, nullptr);
		}

		detail::FreeGPUResource(texture_buffer, logical_device);

		detail::BufferContext context_buffercreation = {};
		context_buffercreation.logical_device = logical_device;
		context_buffercreation.physical_device = physical_device;
		context_buffercreation.graphics_queue = graphics_queue;
		context_buffercreation.command_pool = command_pool;

		detail::BufferData vertexbuffer_info = detail::CreateLocalBuffer<detail::Vertex>(context_buffercreation, vertices_to_render);

		if (vertexbuffer_info.err_code == detail::BufferData::SUCCESS) {
			vertex_buffer = vertexbuffer_info.created_buffer;
			vertex_buffer_memory = vertexbuffer_info.memory_allocated_for_buffer;
		}

		detail::BufferData indexbuffer_info = detail::CreateLocalBuffer<uint32_t>(context_buffercreation, indices);

		if (indexbuffer_info.err_code == detail::BufferData::SUCCESS) {
			index_buffer = indexbuffer_info.created_buffer;
			index_buffer_memory = indexbuffer_info.memory_allocated_for_buffer;
		}

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			detail::BufferData commandbuffer_info = detail::CreateLocalBuffer<VkDrawIndexedIndirectCommand>(context_buffercreation, indirect_commands);

			if (commandbuffer_info.err_code == detail::BufferData::SUCCESS) {
				indirect_command_buffers[i] = commandbuffer_info.created_buffer;
				indirect_command_buffer_memorys[i] = commandbuffer_info.memory_allocated_for_buffer;
			}
			else {
				throw std::runtime_error("Failed to create indirect command buffer.");
			}
		}

		detail::BufferData shaderbuffer_info = detail::CreateLocalBuffer<detail::InstanceData>(context_buffercreation, ProcessInstanceData(NewModelSet));

		if (shaderbuffer_info.err_code == detail::BufferData::SUCCESS) {
			instance_data_buffer = shaderbuffer_info.created_buffer;
			instance_data_buffer_memory = shaderbuffer_info.memory_allocated_for_buffer;
		}

		detail::TextureData merged_texture_data = {};
		uint64_t number_of_textures = 0;

		if (UseWhiteTexture == false) {
			number_of_textures = number_of_indirect_commands;
			merged_texture_data.format = NewModelSet[0].model_data.texture_data.format;
			merged_texture_data.height = NewModelSet[0].model_data.texture_data.height;
			merged_texture_data.width = NewModelSet[0].model_data.texture_data.width;
			merged_texture_data.image_size = NewModelSet[0].model_data.texture_data.image_size * number_of_indirect_commands;
			merged_texture_data.pixels = new stbi_uc[merged_texture_data.image_size];
			uint32_t index = 0;
			for (detail::InstanceModelData model : NewModelSet) {
				for (int i = 0; i < model.model_data.texture_data.image_size; i++) {
					merged_texture_data.pixels[index] = model.model_data.texture_data.pixels[i];
					index += 1;
				}
			}
		}
		else {
			number_of_textures = 1;
			merged_texture_data.format = VK_FORMAT_R8G8B8A8_SRGB;
			merged_texture_data.height = 1;
			merged_texture_data.width = 1;
			merged_texture_data.image_size = 4;
			merged_texture_data.pixels = new stbi_uc[4];
			for (int i = 0; i < 4; i++) {
				merged_texture_data.pixels[i] = 255;
			}
		}

		// Instance Centers Buffer
		std::vector<glm::vec4> instance_centers{};

		for (const detail::InstanceModelData& unique_model : NewModelSet) {
			
			// Get bounding sphere center
			glm::vec3 sum = glm::vec3(0);
			for (const detail::Vertex& vertex : unique_model.model_data.vertices) {
				sum += vertex.position;
			}

			float length = unique_model.model_data.vertices.size();
			glm::vec4 model_center_point = glm::vec4(sum.x / length, sum.y / length, sum.z / length, 1);

			// Apply Model offsets then add centers
			for (const glm::mat4& instance_model_matrix : unique_model.instance_model_matrices) {

				glm::vec4 center_with_offset = instance_model_matrix[3] + model_center_point;
				center_with_offset.w = 1;

				instance_centers.push_back(center_with_offset);
			}
		}

		detail::BufferData instance_centers_info = detail::CreateLocalBuffer<glm::vec4>(context_buffercreation, instance_centers);
		if (instance_centers_info.err_code == detail::BufferData::SUCCESS) {
			instance_centers_buffer = instance_centers_info.created_buffer;
			instance_centers_buffer_memory = instance_centers_info.memory_allocated_for_buffer;
		}

		// Add textures to GPU
		detail::TextureBufferContext context_imagebuffer = {};
		context_imagebuffer.texture_bundle = merged_texture_data;
		context_imagebuffer.logical_device = logical_device;
		context_imagebuffer.physical_device = physical_device;
		context_imagebuffer.command_pool = command_pool;
		context_imagebuffer.data_tiling_mode = VK_IMAGE_TILING_OPTIMAL;
		context_imagebuffer.graphics_queue = graphics_queue;
		context_imagebuffer.memory_flags_required = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		context_imagebuffer.usage_flags = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		context_imagebuffer.number_of_textures = number_of_textures;
		
		texture_buffer = detail::CreateTextureBuffer(context_imagebuffer);
		
		// Update our Graphics Pipeline Descriptor Sets
		detail::Graphic_DescriptorContext context_graphics_update = {};
		context_graphics_update.logical_device = logical_device;
		context_graphics_update.max_frames_in_flight = MAX_FRAMES_IN_FLIGHT;
		context_graphics_update.ubo_size = sizeof(detail::UniformBufferObject);
		context_graphics_update.uniform_buffers = uniform_buffers;
		context_graphics_update.image_view = texture_buffer.image_view;
		context_graphics_update.texture_sampler = texture_sampler;
		context_graphics_update.instance_buffer = instance_data_buffer;
		context_graphics_update.instance_buffer_size = sizeof(detail::InstanceData) * object_count;

		descriptor_sets = detail::UpdateDescriptorSets(context_graphics_update, descriptor_sets);

		detail::Compute_DescriptorContext context_compute_update = {};
		context_compute_update.logical_device = logical_device;
		context_compute_update.max_frames_in_flight = MAX_FRAMES_IN_FLIGHT;
		context_compute_update.indirect_draw_buffers = indirect_command_buffers;
		context_compute_update.indirect_draw_buffer_size = sizeof(VkDrawIndexedIndirectCommand) * number_of_indirect_commands;
		context_compute_update.instance_centers_buffer = instance_centers_buffer;
		context_compute_update.instance_centers_buffer_size = sizeof(glm::vec4) * object_count;
	
		descriptor_sets = detail::UpdateComputeUniqueDescriptor(context_compute_update, descriptor_sets);

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
		swapchain_context.OLD_depth_buffer = depth_buffer;

		detail::RecreateSwapchainData out_data = detail::RecreateSwapchain(swapchain_context);

		swapchain = out_data.swapchain_data.swapchain;
		extent = out_data.swapchain_data.swapchain_extent;

		std::cout << "Recreated swapchain with new extent: " << extent.width << " X " << extent.height << "." << std::endl;

		framebuffers = out_data.framebuffers;
		swapchain_images = out_data.swapchain_images;
		swapchain_image_views = out_data.swapchain_image_views;
		depth_buffer = out_data.depth_buffer;
	}
}// namespace renderer