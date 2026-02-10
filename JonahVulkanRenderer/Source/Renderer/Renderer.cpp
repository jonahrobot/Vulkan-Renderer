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
#include "NewDetail/VkSceneProcesser.h"

namespace renderer {

	// Unnamed namespace to show functions below are pure Utility with no internal state. 
	// Cannot see or access private data of Renderer class.
	namespace {

		UBOData GetNextUBO(VkExtent2D SwapchainExtent, glm::mat4 CameraPosition) {

			UBOData ubo = {};

			ubo.proj = glm::perspective(glm::radians(45.0f), SwapchainExtent.width / (float)SwapchainExtent.height, 0.01f, 100.0f);
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

	} // namespace util

	static void FramebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto renderer = reinterpret_cast<Renderer*>(glfwGetWindowUserPointer(window));
		renderer->framebuffer_resized = true;
	}

	Renderer::Renderer(int ScreenWidth, int ScreenHeight) {

		// Inital setup
		mesh_count = 0;
		unique_mesh_count = 0;
		const char* vertex_shader_path = "shaders/vert.spv";
		const char* fragment_shader_path = "shaders/frag.spv";
		const char* compute_shader_path = "shaders/cull.spv";

		// GLFW setup
		window = device::CreateVulkanWindow("OpenUSD Renderer", ScreenWidth, ScreenHeight);
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
		graphics_pipeline = pipeline::CreateGraphicsPipeline(logical_device, pipeline_layout, render_pass, vertex_shader_path, fragment_shader_path);
		compute_pipeline = pipeline::CreateComputePipeline(logical_device, pipeline_layout, compute_shader_path);

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
		cmd_begin_debug = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(vkGetInstanceProcAddr(vulkan_instance, "vkCmdBeginDebugUtilsLabelEXT"));
		cmd_end_debug = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(vkGetInstanceProcAddr(vulkan_instance, "vkCmdEndDebugUtilsLabelEXT"));
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
		data::DestroyBuffer(logical_device, mesh_centers_buffer);
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

	void Renderer::RecordComputeCommands(uint32_t CurrentFrame, bool FrustumCull) {

		VkCommandBuffer command_buffer = compute_command_buffers[CurrentFrame];

		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = 0;
		begin_info.pInheritanceInfo = nullptr;

		if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS) {
			throw std::runtime_error("Failed to begin recording compute command buffer.");
		}

		draw::DEBUG_StartLabelCommand(cmd_begin_debug, command_buffer, "Compute Workload", {0.455f, 0.259f, 0.325f, 1.0f});

		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute_pipeline);
		vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_layout, 0, 1, &descriptor_sets[CurrentFrame], 0, 0);

		if (FrustumCull) {
			vkCmdDispatch(command_buffer, mesh_count / 64, 1, 1);
		}
		else {
			vkCmdDispatch(command_buffer, 0, 1, 1); // Do not run compute commands.
		}

		draw::DEBUG_EndLabelCommand(cmd_end_debug, command_buffer);

		if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
			throw std::runtime_error("Failed to record command buffer.");
		}
	}

	void Renderer::RecordGraphicsCommands(uint32_t CurrentFrame, uint32_t ImageIndex) {

		VkCommandBuffer command_buffer = graphics_command_buffers[CurrentFrame];

		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = 0;
		begin_info.pInheritanceInfo = nullptr;

		if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS) {
			throw std::runtime_error("Failed to begin recording command buffer.");
		}

		draw::DEBUG_StartLabelCommand(cmd_begin_debug, command_buffer, "Render Pass", { 0.016f, 0.565f, 1.0f, 1.0f });

		VkRenderPassBeginInfo render_pass_info{};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_info.renderPass = render_pass;
		render_pass_info.framebuffer = framebuffers[ImageIndex];
		render_pass_info.renderArea.offset = { 0,0 };
		render_pass_info.renderArea.extent = swapchain_extent;

		std::array<VkClearValue, 2> clear_values;
		clear_values[0].color = { {0.0f,0.0f,0.0f,1.0f} };
		clear_values[1].depthStencil = { 1.0f,0 };

		render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
		render_pass_info.pClearValues = clear_values.data();

		vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(swapchain_extent.width);
		viewport.height = static_cast<float>(swapchain_extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(command_buffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0,0 };
		scissor.extent = swapchain_extent;
		vkCmdSetScissor(command_buffer, 0, 1, & scissor);

		if (vertex_buffer.ByteSize != 0) {

			VkBuffer vertex_buffers[] = { vertex_buffer.Buffer };
			VkDeviceSize offsets[] = { 0 };

			vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);
			vkCmdBindIndexBuffer(command_buffer, index_buffer.Buffer, 0, VK_INDEX_TYPE_UINT32);
			vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_sets[current_frame], 0, nullptr);

			uint32_t size_of_command = sizeof(VkDrawIndexedIndirectCommand);

			for (uint32_t x = 0; x < unique_mesh_count; x++) {
				vkCmdDrawIndexedIndirect(command_buffer, indirect_command_buffers[current_frame].Buffer, x * size_of_command, 1, size_of_command);
			}
		}

		vkCmdEndRenderPass(command_buffer);

		draw::DEBUG_EndLabelCommand(cmd_end_debug, command_buffer);

		if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
			throw std::runtime_error("Failed to record command buffer.");
		}
	}

	void Renderer::Draw(glm::mat4 CameraPosition, bool FrustumCull) {

		vkWaitForFences(logical_device, 1, &in_flight_fences[current_frame], VK_TRUE, UINT64_MAX);
		vkWaitForFences(logical_device, 1, &compute_in_flight_fences[current_frame], VK_TRUE, UINT64_MAX);

		UBOData current_ubo_data = GetNextUBO(swapchain_extent, CameraPosition);
		memcpy(uniform_buffers[current_frame].BufferMapped, &current_ubo_data, sizeof(UBOData));

		vkResetFences(logical_device, 1, &compute_in_flight_fences[current_frame]);
		vkResetCommandBuffer(compute_command_buffers[current_frame], 0);
		
		// Compute Cull
		RecordComputeCommands(current_frame, FrustumCull);
		
		VkSubmitInfo submit_info_compute{};
		submit_info_compute.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info_compute.commandBufferCount = 1;
		submit_info_compute.pCommandBuffers = &compute_command_buffers[current_frame];
		submit_info_compute.signalSemaphoreCount = 1;
		submit_info_compute.pSignalSemaphores = &compute_finished_semaphores[current_frame];

		if (vkQueueSubmit(compute_queue, 1, &submit_info_compute, compute_in_flight_fences[current_frame]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to submit compute command buffer.");
		}

		// Prep for draw
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
		
		// Graphics Draw
		RecordGraphicsCommands(current_frame, image_index);

		VkSemaphore wait_semaphores[] = { compute_finished_semaphores[current_frame], image_available_semaphores[current_frame] };
		VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT , VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSemaphore signal_semaphores[] = { render_finished_semaphores[image_index] };

		VkSubmitInfo submit_info{};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.pWaitSemaphores = wait_semaphores;
		submit_info.pWaitDstStageMask = wait_stages;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &graphics_command_buffers[current_frame];
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = signal_semaphores;

		if (vkQueueSubmit(graphics_queue, 1, &submit_info, in_flight_fences[current_frame]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to submit draw command buffer.");
		}

		// Present
		VkPresentInfoKHR present_info{};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = signal_semaphores;
		present_info.swapchainCount = 1;
		present_info.pSwapchains = &swapchain;
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

	void Renderer::UpdateModelSet(std::vector<MeshInstances> NewModelSet, bool UseWhiteTexture) {

		vkDeviceWaitIdle(logical_device);

		// Clear old data
		data::DestroyBuffer(logical_device, vertex_buffer);
		data::DestroyBuffer(logical_device, index_buffer);
		data::DestroyBuffer(logical_device, mesh_centers_buffer);
		data::DestroyBuffer(logical_device, instance_data_buffer);
	
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			data::DestroyBuffer(logical_device, indirect_command_buffers[i]);
			data::DestroyBuffer(logical_device, should_draw_buffers[i]);
		}

		// Get new data
		scene::SceneParser parser = scene::SceneParser(NewModelSet);

		std::vector<Vertex> vertex_buffer_data = parser.GetSceneVertices();
		std::vector<uint32_t> index_buffer_data = parser.GetSceneIndices();
		std::vector<InstanceData> instance_data = parser.GetInstanceData();
		std::vector<glm::vec4> model_centers_data = parser.GetModelCenters();
		std::vector<VkDrawIndexedIndirectCommand> indirect_commands = parser.GetDrawCommands();
		std::vector<uint32_t> should_draw_flags(mesh_count, 0);

		mesh_count = parser.GetMeshCount();
		unique_mesh_count = indirect_commands.size();

		// Load new data to GPU
		data::BaseBufferContext ctx = {};
		ctx.LogicalDevice = logical_device;
		ctx.PhysicalDevice = physical_device;
		ctx.GraphicsQueue = graphics_queue;
		ctx.CommandPool = graphics_command_pool;

		VkBufferUsageFlags transfer_bit = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		VkBufferUsageFlags indirect_bit = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
		VkBufferUsageFlags storage_bit = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		VkBufferUsageFlags vertex_bit = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		VkBufferUsageFlags index_bit = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

		vertex_buffer = data::CreateBuffer(vertex_buffer_data.data(), vertex_buffer_data.size(), transfer_bit | vertex_bit, ctx);
		index_buffer = data::CreateBuffer(index_buffer_data.data(), index_buffer_data.size(), transfer_bit | index_bit, ctx);
		instance_data_buffer = data::CreateBuffer(instance_data.data(), instance_data.size(), storage_bit | transfer_bit, ctx);
		mesh_centers_buffer = data::CreateBuffer(model_centers_data.data(), model_centers_data.size(), storage_bit | transfer_bit, ctx);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			indirect_command_buffers[i] = data::CreateBuffer(indirect_commands.data(), indirect_commands.size(), indirect_bit | storage_bit | transfer_bit, ctx);
			should_draw_buffers[i] = data::CreateBuffer(should_draw_flags.data(), should_draw_flags.size(), storage_bit | transfer_bit, ctx);
		}

		data::UpdateDescriptorSets(descriptor_sets, logical_device, instance_data_buffer, mesh_centers_buffer, uniform_buffers, should_draw_buffers, indirect_command_buffers);
	}

	GLFWwindow* Renderer::Get_Window() {
		return window;
	}

	void Renderer::RecreateSwapchainHelper() {

		int width = 0, height = 0;
		glfwGetFramebufferSize(window, &width, &height);
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(logical_device);

		// Cleanup old swapchain
		draw::DestroyDepthBuffer(logical_device, depth_buffer);

		for (auto framebuffer : framebuffers) {
			vkDestroyFramebuffer(logical_device, framebuffer, nullptr);
		}

		for (size_t i = 0; i < swapchain_image_views.size(); i++) {
			vkDestroyImageView(logical_device, swapchain_image_views[i], nullptr);
		}
		vkDestroySwapchainKHR(logical_device, swapchain, nullptr);

		// Create new swapchain
		swapchain::SwapchainOptions swapchain_options = swapchain::QuerySwapchainSupport(physical_device, vulkan_surface);
		
		swapchain_format = swapchain::ChooseFormat(swapchain_options);
		swapchain_present_mode = swapchain::ChoosePresentMode(swapchain_options);
		swapchain_extent = swapchain::ChooseExtent(swapchain_options, window);
		swapchain_image_count = swapchain::ChooseImageCount(swapchain_options);
		
		swapchain = swapchain::CreateSwapchain(logical_device, vulkan_surface, swapchain_format, swapchain_present_mode, swapchain_extent, swapchain_image_count, swapchain_options, queues_supported);

		swapchain_images = swapchain::CreateSwapchainImages(logical_device, swapchain);
		swapchain_image_views = swapchain::CreateSwapchainViews(logical_device, swapchain_format.format, swapchain_images);

		depth_buffer = draw::CreateDepthBuffer(logical_device, physical_device, swapchain_extent);
		framebuffers = draw::CreateFramebuffers(logical_device, depth_buffer, render_pass, swapchain_extent, swapchain_image_views);
	}
}// namespace renderer