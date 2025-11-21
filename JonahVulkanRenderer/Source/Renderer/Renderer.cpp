
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

		Renderer::UniformBufferObject GetNextUBO(VkExtent2D swapchain_extent) {

			static auto start_time = std::chrono::high_resolution_clock::now();

			auto current_time = std::chrono::high_resolution_clock::now();
			float time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();

			Renderer::UniformBufferObject ubo{};
			ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // This handles the objects position relative to the world space
			//ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)); // This tells us the cameras position
			ubo.proj = glm::perspective(glm::radians(45.0f), swapchain_extent.width / (float)swapchain_extent.height, 0.1f, 10.0f); // This helps us project the point to the viewport

			ubo.proj[1][1] *= -1;

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

		struct ModelData {
			std::vector<detail::Vertex> vertices_to_render;
			std::vector<uint32_t>indices;
		};
		ModelData LoadModel(std::string ModelPath) {

			ModelData new_model;

			tinyobj::attrib_t attrib;
			std::vector<tinyobj::shape_t> shapes;
			std::vector<tinyobj::material_t> materials;
			std::string err;
			std::string warn;

			if (tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, ModelPath.c_str()) == false) {
				throw std::runtime_error(err);
			}

			std::unordered_map<detail::Vertex, uint32_t> unique_vertices{};

			for (const auto& shape : shapes) {
				for (const auto& index : shape.mesh.indices) {
					detail::Vertex vertex{};

					vertex.position = {
						attrib.vertices[3 * index.vertex_index + 0],
						attrib.vertices[3 * index.vertex_index + 1],
						attrib.vertices[3 * index.vertex_index + 2]
					};

					vertex.tex_coord = {
						attrib.texcoords[2 * index.texcoord_index + 0],
						1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
					};

					vertex.color = { 1.0f, 1.0f, 1.0f };

					// If vertex not been acounted for then add it!
					if (unique_vertices.count(vertex) == 0) {
						unique_vertices[vertex] = static_cast<uint32_t> (new_model.vertices_to_render.size());
						new_model.vertices_to_render.push_back(vertex);
					}

					// Find vertex's index and add it to indices array
					new_model.indices.push_back(unique_vertices[vertex]);
				}
			}

			return new_model;
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
		vkGetDeviceQueue(logical_device, physical_device_data.queues_supported.graphicsFamily.value(), 0, &graphics_queue);
		vkGetDeviceQueue(logical_device, physical_device_data.queues_supported.presentFamily.value(), 0, &present_queue);

		// Create render pass
		render_pass = CreateRenderPass(logical_device, swapchain_info.swapchain_image_format, physical_device);

		// Create descriptor set for camera UBO, used in graphics pipeline creation
		descriptor_set_layout = detail::CreateDescriptorLayout({logical_device});

		// Create graphics pipeline
		detail::GraphicsPipelineContext context_graphics_pipeline = {};
		context_graphics_pipeline.logical_device = logical_device;
		context_graphics_pipeline.render_pass = render_pass;
		context_graphics_pipeline.swapchain_extent = extent;
		context_graphics_pipeline.vertex_descriptor_set_layout = descriptor_set_layout;

		detail::GraphicsPipelineData pipeline_info = detail::CreateGraphicsPipeline(context_graphics_pipeline);
		graphics_pipeline = pipeline_info.pipeline;
		graphics_pipeline_layout = pipeline_info.layout;

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
		command_pool = detail::CreateCommandPool(logical_device, physical_device_data.queues_supported.graphicsFamily.value());
		command_buffers = detail::CreateCommandBuffers(MAX_FRAMES_IN_FLIGHT, logical_device, command_pool);

		// Create Texture Image
		detail::TextureBundle rock_texture = detail::LoadTextureImage(TEXTURE_PATH.c_str());

		detail::ImageObjectContext context_imagebuffer = {};
		context_imagebuffer.texture_bundle = rock_texture;
		context_imagebuffer.logical_device = logical_device;
		context_imagebuffer.physical_device = physical_device;
		context_imagebuffer.command_pool = command_pool;
		context_imagebuffer.data_tiling_mode = VK_IMAGE_TILING_OPTIMAL;
		context_imagebuffer.graphics_queue = graphics_queue;
		context_imagebuffer.memory_flags_required = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		context_imagebuffer.usage_flags = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

		texture_0 = detail::CreateGPUResource(context_imagebuffer);

		detail::FreeTextureBundle(rock_texture);

		// Load Model
		ModelData model_0 = LoadModel(MODEL_PATH);
		vertices_to_render = model_0.vertices_to_render;
		indices = model_0.indices;

		// Create Texture Sampler
		texture_sampler = CreateTextureSampler(physical_device, logical_device);

		// Create Vertex Buffer
		detail::VertexBufferContext context_vertexbuffer = {};
		context_vertexbuffer.vertices_to_render = vertices_to_render;
		context_vertexbuffer.logical_device = logical_device;
		context_vertexbuffer.physical_device = physical_device;
		context_vertexbuffer.graphics_queue = graphics_queue;
		context_vertexbuffer.command_pool = command_pool;

		detail::BufferData vertexbuffer_info = detail::CreateVertexBuffer(context_vertexbuffer);
		vertex_buffer = vertexbuffer_info.created_buffer;
		vertex_buffer_memory = vertexbuffer_info.memory_allocated_for_buffer;

		// Create Index Buffer
		detail::IndexBufferContext context_indexbuffer = {};
		context_indexbuffer.indices = indices;
		context_indexbuffer.logical_device = logical_device;
		context_indexbuffer.physical_device = physical_device;
		context_indexbuffer.graphics_queue = graphics_queue;
		context_indexbuffer.command_pool = command_pool;

		detail::BufferData indexbuffer_info = detail::CreateIndexBuffer(context_indexbuffer);
		index_buffer = indexbuffer_info.created_buffer;
		index_buffer_memory = indexbuffer_info.memory_allocated_for_buffer;

		// Create UBO for Vertex
		detail::UniformBufferContext context_ubo = {};
		context_ubo.logical_device = logical_device;
		context_ubo.max_frames_in_flight = MAX_FRAMES_IN_FLIGHT;
		context_ubo.physical_device = physical_device;
		context_ubo.ubo_size = sizeof(UniformBufferObject);

		detail::UniformBufferData ubo_info = detail::CreateUniformBuffers(context_ubo);
		uniform_buffers = ubo_info.uniform_buffers;
		uniform_buffers_memory = ubo_info.uniform_buffers_memory;
		uniform_buffers_mapped = ubo_info.uniform_buffers_mapped;

		// Create Descriptor Pool
		detail::DescriptorPoolContext context_pool = {};
		context_pool.logical_device = logical_device;
		context_pool.max_frames_in_flight = MAX_FRAMES_IN_FLIGHT;

		descriptor_pool = detail::CreateDescriptorPool(context_pool);

		// Create Descriptor Sets to link UBO to GPU
		detail::DescriptorSetContext context_descriptor_set = {};
		context_descriptor_set.descriptor_pool = descriptor_pool;
		context_descriptor_set.descriptor_set_layout = descriptor_set_layout;
		context_descriptor_set.logical_device = logical_device;
		context_descriptor_set.max_frames_in_flight = MAX_FRAMES_IN_FLIGHT;
		context_descriptor_set.ubo_size = sizeof(UniformBufferObject);
		context_descriptor_set.uniform_buffers = uniform_buffers;
		context_descriptor_set.image_view = texture_0.image_view;
		context_descriptor_set.texture_sampler = texture_sampler;

		descriptor_sets = detail::CreateDescriptorSets(context_descriptor_set);

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

		vkDestroySampler(logical_device, texture_sampler, nullptr);

		detail::FreeGPUResource(texture_0, logical_device);
		detail::FreeGPUResource(depth_buffer, logical_device);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroyBuffer(logical_device, uniform_buffers[i], nullptr);
			vkFreeMemory(logical_device, uniform_buffers_memory[i], nullptr);
		}

		vkDestroyDescriptorPool(logical_device, descriptor_pool, nullptr);

		vkDestroyDescriptorSetLayout(logical_device, descriptor_set_layout, nullptr);

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

		vkDestroyBuffer(logical_device, index_buffer, nullptr);
		vkFreeMemory(logical_device, index_buffer_memory, nullptr);

		vkDestroyBuffer(logical_device, vertex_buffer, nullptr);
		vkFreeMemory(logical_device, vertex_buffer_memory, nullptr);

		vkDestroyDevice(logical_device, nullptr);
		vkDestroySurfaceKHR(vulkan_instance, vulkan_surface, nullptr);
		vkDestroyInstance(vulkan_instance, nullptr); // Cleanup instance LAST in Vulkan Cleanup

		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void Renderer::Draw(glm::mat4 CameraPosition) {

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

		// Update camera position (temp)
		UniformBufferObject current_ubo_data = GetNextUBO(extent);
		current_ubo_data.view = CameraPosition;
		memcpy(uniform_buffers_mapped[current_frame], &current_ubo_data, sizeof(current_ubo_data));

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