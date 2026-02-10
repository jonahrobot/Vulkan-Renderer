#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#include <unordered_map>
#include <iostream>

#include "Renderer.h"
#include "NewDetail/VkCommon.h"
#include "NewDetail/VkDeviceSetup.h"
#include "NewDetail/VkSwapchainSetup.h"
#include "NewDetail/VkPipelineSetup.h"
#include "NewDetail/VkDataSetup.h"
#include "NewDetail/VkDrawSetup.h"

namespace renderer {

	// Unnamed namespace to show functions below are pure Utility with no internal state. 
	// Cannot see or access private data of Renderer class.
	namespace {


		std::vector<detail::InstanceData> ProcessInstanceData(const std::vector<detail::MeshInstances>& NewModelSet) {

			std::vector<detail::InstanceData> instance_data = {};

			for (detail::MeshInstances model_data : NewModelSet) {

				for (uint32_t i = 0; i < model_data.instance_count; i++) {

					detail::InstanceData new_instance;

					new_instance.model = model_data.instance_model_matrices[i];
					new_instance.array_index.x = 0;

					instance_data.push_back(new_instance);
				}
			}

			return instance_data;
		}

		detail::UBOData GetNextUBO(VkExtent2D Extent, glm::mat4 CameraPosition) {

			detail::UBOData ubo = {};

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

		std::vector<VkDrawIndexedIndirectCommand> RecordIndirectCommands(std::vector<Vertex>& VerticeToRender, std::vector<uint32_t>& Indices, uint32_t& NumberOfMeshes, const std::vector<detail::MeshInstances>& NewModelSet) {
			std::vector<VkDrawIndexedIndirectCommand> indirect_commands;

			uint32_t m = 0;
			NumberOfMeshes = 0;
			VerticeToRender.clear();
			Indices.clear();

			for (detail::MeshInstances model_usage_data : NewModelSet) {

				detail::Mesh model = model_usage_data.model_data;

				bool no_data = model.vertices.size() == 0 || model.indices.size() == 0;
				if (no_data) continue;

				NumberOfMeshes += model_usage_data.instance_count;

				uint32_t offset = static_cast<uint32_t>(VerticeToRender.size());

				for (const Vertex& v : model.vertices) {
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

		// Inital setup
		mesh_count = 0;
		unique_mesh_count = 0;

		// GLFW setup
		window = device::CreateVulkanWindow("OpenUSD Renderer", WIDTH, HEIGHT);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, FramebufferResizeCallback);

		uint32_t number_of_extentions = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&number_of_extentions);

		for (int i = 0; i < number_of_extentions; i++) {
			InstanceExtensionsToSupport.push_back(glfwExtensions[i]);
		}
		
		// Device setup
		vulkan_instance = device::CreateVulkanInstance(UseValidationLayers, ValidationLayersToSupport, InstanceExtensionsToSupport);
		vulkan_surface = device::CreateVulkanSurface(vulkan_instance, window);
		queues_supported = device::FindSupportedQueues(physical_device, vulkan_surface);
		physical_device = device::PickPhysicalDevice(vulkan_instance, vulkan_surface, DeviceExtensionsToSupport);

		device::LogicalDeviceContext context_logical = {};
		context_logical.PhysicalDevice = physical_device;
		context_logical.SupportedQueues = queues_supported;
		context_logical.UseValidationLayers = UseValidationLayers;
		context_logical.DeviceExtensionsToSupport = DeviceExtensionsToSupport;
		context_logical.ValidationLayersToSupport = ValidationLayersToSupport;

		logical_device = device::CreateLogicalDevice(context_logical);

		// Swapchain setup
		swapchain::SwapchainOptions swapchain_options = swapchain::QuerySwapchainSupport(physical_device, vulkan_surface);

		swapchain_format = swapchain::ChooseFormat(swapchain_options);
		swapchain_present_mode = swapchain::ChoosePresentMode(swapchain_options);
		swapchain_extent = swapchain::ChooseExtent(swapchain_options, window);
		swapchain_image_count = swapchain::ChooseImageCount(swapchain_options);

		swapchain = swapchain::CreateSwapchain(logical_device, vulkan_surface, swapchain_format, swapchain_present_mode, swapchain_extent, swapchain_image_count, swapchain_options, queues_supported);
		
		swapchain_images = swapchain::CreateSwapchainImages(logical_device, swapchain);
		swapchain_image_views = swapchain::CreateSwapchainViews(logical_device, swapchain_format.format, swapchain_images);

		for (size_t i = 0; i < swapchain_images.size(); i++) {
			render_finished_semaphores.push_back(draw::CreateVulkanSemaphore(logical_device));
		}

		// Pipeline setup
		depth_buffer = draw::CreateDepthBuffer(logical_device, physical_device, swapchain_extent);
		render_pass = pipeline::CreateRenderPass(logical_device, physical_device, swapchain_format.format, depth_buffer.ImageFormat);
		framebuffers = draw::CreateFramebuffers(logical_device, depth_buffer, render_pass, swapchain_extent, swapchain_image_views);

		descriptor_layout = pipeline::CreateDescriptorLayout(logical_device);
		descriptor_pool = pipeline::CreateDescriptorPool(logical_device);
		descriptor_sets = pipeline::CreateDescriptorSets(logical_device, descriptor_layout, descriptor_pool);

		pipeline_layout = pipeline::CreatePipelineLayout(logical_device, descriptor_layout);
		graphics_pipeline = pipeline::CreateGraphicsPipeline(logical_device, pipeline_layout, render_pass, VERTEX_SHADER, FRAGMENT_SHADER);
		compute_pipeline = pipeline::CreateComputePipeline(logical_device, pipeline_layout, COMPUTE_SHADER);

		// Draw setup
		graphics_command_pool = draw::CreateCommandPool(logical_device, queues_supported.graphics_compute_family.value());
		graphics_command_buffers = draw::CreateCommandBuffers(logical_device, graphics_command_pool, MAX_FRAMES_IN_FLIGHT);

		compute_command_pool = draw::CreateCommandPool(logical_device, queues_supported.graphics_compute_family.value());
		compute_command_buffers = draw::CreateCommandBuffers(logical_device, compute_command_pool, MAX_FRAMES_IN_FLIGHT);

		vkGetDeviceQueue(logical_device, queues_supported.graphics_compute_family.value(), 0, &graphics_queue);
		vkGetDeviceQueue(logical_device, queues_supported.present_family.value(), 0, &present_queue);
		vkGetDeviceQueue(logical_device, queues_supported.graphics_compute_family.value(), 0, &compute_queue);

		// Per frame setup
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {

			// Graphics sync objects
			image_available_semaphores.push_back(draw::CreateVulkanSemaphore(logical_device));
			in_flight_fences.push_back(draw::CreateVulkanFence(logical_device));

			// Compute sync objects
			compute_in_flight_fences.push_back(draw::CreateVulkanFence(logical_device));
			compute_finished_semaphores.push_back(draw::CreateVulkanSemaphore(logical_device));

			// UBO for graphics and compute
			uniform_buffers[i] = data::CreateUBO(logical_device, physical_device, sizeof(UBOData));
		}

		// Debug setup
		pfn_CmdBeginDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(vkGetInstanceProcAddr(vulkan_instance, "vkCmdBeginDebugUtilsLabelEXT"));
		pfn_CmdEndDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(vkGetInstanceProcAddr(vulkan_instance, "vkCmdEndDebugUtilsLabelEXT"));
	}

	Renderer::~Renderer() {

		vkDeviceWaitIdle(logical_device);

		// Cleanup per frame data
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {

			// Graphics sync objects
			vkDestroySemaphore(logical_device, image_available_semaphores[i], nullptr);
			vkDestroyFence(logical_device, in_flight_fences[i], nullptr);

			// Compute sync objects
			vkDestroySemaphore(logical_device, compute_finished_semaphores[i], nullptr);
			vkDestroyFence(logical_device, compute_in_flight_fences[i], nullptr);

			// UBO for graphics and compute
			data::DestroyUBO(logical_device, uniform_buffers[i]);

			// SSBO for graphics and compute
			data::DestroyBuffer(logical_device, indirect_command_buffers[i]);
			data::DestroyBuffer(logical_device, should_draw_buffers[i]);
		}

		// Cleanup render data
		data::DestroyBuffer(logical_device, vertex_buffer);
		data::DestroyBuffer(logical_device, index_buffer);
		data::DestroyBuffer(logical_device, instance_centers_buffer);
		data::DestroyBuffer(logical_device, instance_data_buffer);

		// Cleanup draw framework
		vkDestroyCommandPool(logical_device, graphics_command_pool, nullptr);
		vkDestroyCommandPool(logical_device, compute_command_pool, nullptr);

		// Cleanup pipeline
		vkDestroyPipeline(logical_device, graphics_pipeline, nullptr);
		vkDestroyPipeline(logical_device, compute_pipeline, nullptr);
		vkDestroyPipelineLayout(logical_device, pipeline_layout, nullptr);

		vkDestroyDescriptorPool(logical_device, descriptor_pool, nullptr);
		vkDestroyDescriptorSetLayout(logical_device, descriptor_layout, nullptr);

		for (auto framebuffer : framebuffers) {
			vkDestroyFramebuffer(logical_device, framebuffer, nullptr);
		}
		draw::DestroyDepthBuffer(logical_device, depth_buffer);
		vkDestroyRenderPass(logical_device, render_pass, nullptr);

		// Cleanup swapchain
		for (size_t i = 0; i < swapchain_image_views.size(); i++) {
			vkDestroyImageView(logical_device, swapchain_image_views[i], nullptr);
			vkDestroyImage(logical_device, swapchain_images[i], nullptr);
			vkDestroySemaphore(logical_device, render_finished_semaphores[i], nullptr);
		}
		vkDestroySwapchainKHR(logical_device, swapchain, nullptr);
		
		// Cleanup device
		vkDestroyDevice(logical_device, nullptr);
		vkDestroySurfaceKHR(vulkan_instance, vulkan_surface, nullptr);

		// Destroy instance - Last Vulkan VkDestroy command to call.
		vkDestroyInstance(vulkan_instance, nullptr);

		// Cleanup GLFW
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void Renderer::Draw(glm::mat4 CameraPosition, bool FrustumCull) {

		/// PREP DRAW

		// COMPUTE WORKLOAD
		vkWaitForFences(logical_device, 1, &in_flight_fences[current_frame], VK_TRUE, UINT64_MAX);
		vkWaitForFences(logical_device, 1, &compute_in_flight_fences[current_frame], VK_TRUE, UINT64_MAX);

		// Get Camera position
		detail::UBOData current_ubo_data = GetNextUBO(extent, CameraPosition);

		memcpy(uniform_buffers[current_frame].buffer_mapped, &current_ubo_data, sizeof(detail::UBOData));

		vkResetFences(logical_device, 1, &compute_in_flight_fences[current_frame]);

		vkResetCommandBuffer(compute_command_buffers[current_frame], 0);

		detail::Compute_CommandRecordingContext compute_command_context{};
		compute_command_context.command_buffer = compute_command_buffers[current_frame];
		compute_command_context.compute_pipeline = compute_pipeline;
		compute_command_context.current_descriptor_set = descriptor_sets[current_frame];
		if (FrustumCull) {
			compute_command_context.instance_count = mesh_count;
		}
		else {
			compute_command_context.instance_count = 0;
		}
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

		vkResetCommandBuffer(graphics_command_buffers[current_frame], 0);
		
		/// DRAW
		detail::CommandRecordingContext command_context{};
		command_context.framebuffers = framebuffers;
		command_context.render_pass = render_pass;
		command_context.graphics_pipeline = graphics_pipeline;
		command_context.command_buffer = graphics_command_buffers[current_frame];
		command_context.current_descriptor_set = descriptor_sets[current_frame];
		command_context.image_write_index = image_index;
		command_context.swapchain_extent = extent;
		command_context.vertex_buffer = vertex_buffer;
		command_context.index_buffer = index_buffer;
		command_context.unique_mesh_count = unique_mesh_count;
		command_context.indirect_command_buffer = indirect_command_buffers[current_frame];
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
		submit_info.pCommandBuffers = &graphics_command_buffers[current_frame];

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
	void Renderer::UpdateModelSet(std::vector<detail::MeshInstances> NewModelSet, bool UseWhiteTexture) {

		vkDeviceWaitIdle(logical_device);

		DestroyBuffer(logical_device, vertex_buffer);
		DestroyBuffer(logical_device, index_buffer);
		DestroyBuffer(logical_device, instance_centers_buffer);
		DestroyBuffer(logical_device, instance_data_buffer);
	
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			DestroyBuffer(logical_device, indirect_command_buffers[i]);
			DestroyBuffer(logical_device, should_draw_buffers[i]);
		}

		std::vector<detail::Vertex> vertices_to_render;
		std::vector<uint32_t>indices;

		std::vector<VkDrawIndexedIndirectCommand> indirect_commands = RecordIndirectCommands(vertices_to_render, indices, mesh_count, NewModelSet);
		unique_mesh_count = indirect_commands.size();

		// Create buffers

		detail::BufferContext context_buffercreation = {};
		context_buffercreation.logical_device = logical_device;
		context_buffercreation.physical_device = physical_device;
		context_buffercreation.graphics_queue = graphics_queue;
		context_buffercreation.graphics_command_pool = graphics_command_pool;

		vertex_buffer = detail::CreateLocalBuffer<detail::Vertex>(context_buffercreation, vertices_to_render, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		index_buffer = detail::CreateLocalBuffer<uint32_t>(context_buffercreation, indices, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

		VkBufferUsageFlags commandbuffer_usage_flags = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			indirect_command_buffers[i] = detail::CreateLocalBuffer<VkDrawIndexedIndirectCommand>(context_buffercreation, indirect_commands, commandbuffer_usage_flags);
		}

		std::vector<uint32_t> should_draw_flags(mesh_count, 0);
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			should_draw_buffers[i] = detail::CreateLocalBuffer<uint32_t>(context_buffercreation, should_draw_flags, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		}

		instance_data_buffer = detail::CreateLocalBuffer<detail::InstanceData>(context_buffercreation, ProcessInstanceData(NewModelSet), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);


		// Instance Centers Buffer
		std::vector<glm::vec4> instance_centers{};

		for (const detail::MeshInstances& unique_model : NewModelSet) {
			
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

		instance_centers_buffer = detail::CreateLocalBuffer<glm::vec4>(context_buffercreation, instance_centers, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

		// Update our Graphics Pipeline Descriptor Sets
		detail::Graphic_DescriptorContext context_graphics_update = {};
		context_graphics_update.logical_device = logical_device;
		context_graphics_update.uniform_buffers = uniform_buffers;
		context_graphics_update.instance_buffer = instance_data_buffer;
		context_graphics_update.should_draw_flags_buffer = should_draw_buffers;

		descriptor_sets = detail::UpdateDescriptorSets(context_graphics_update, descriptor_sets);

		detail::Compute_DescriptorContext context_compute_update = {};
		context_compute_update.logical_device = logical_device;
		context_compute_update.indirect_draw_buffers = indirect_command_buffers;
		context_compute_update.instance_centers = instance_centers_buffer;

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