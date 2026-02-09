#pragma once

#include <optional>
#include <vector>

#include "RendererDetail_Common.h"


// Internal Helper functions for renderer.cpp
// Public Rendering API available at: "Renderer.h"
namespace renderer::detail {

	// Detail namespace seperates implementation logic of utility functions into their own classes.
	// Renderer::detail naming convention tells us these are specific implementations for our renderer class.

	// All below functions construct parts of the Vulkan Renderer used in "Renderer.cpp"

#pragma region Shared Structs
	struct Pipeline {
		VkPipeline pipeline;
		VkPipelineLayout layout;
	};

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphics_compute_family;
		std::optional<uint32_t> present_family;
	};

	struct Buffer {
		VkBuffer buffer;
		VkDeviceMemory memory;
		uint64_t byte_size = 0;
	};

	struct BufferMapped {
		Buffer buffer;
		void* buffer_mapped;
	};

	struct Mesh {
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
	};

	struct MeshInstances {
		Mesh model_data;
		uint32_t instance_count = 0;
		std::vector<glm::mat4> instance_model_matrices;
	};
#pragma endregion

#pragma region Vulkan Instance
	
#pragma endregion

#pragma region Physical Device

#pragma endregion

#pragma region Logical Device
	// Implemented in "LogicalDevice.cpp"

#pragma endregion

#pragma region Swapchain
	// Implemented in "Swapchain.cpp"


	struct RecreateSwapchainContext {
		SwapchainContext swapchain_creation_data;
		VkRenderPass render_pass;

		VkSwapchainKHR OLD_swapchain;
		GPUResource OLD_depth_buffer;
		std::vector<VkImageView> OLD_swapchain_image_views;
		std::vector<VkFramebuffer> OLD_framebuffers;
	};
	struct RecreateSwapchainData {
		SwapchainData swapchain_data;
		GPUResource depth_buffer;
		std::vector<VkImage> swapchain_images;
		std::vector<VkImageView> swapchain_image_views;
		std::vector<VkFramebuffer> framebuffers;
	};
	RecreateSwapchainData RecreateSwapchain(RecreateSwapchainContext Context);
#pragma endregion

#pragma region Command Buffers


	struct CommandRecordingContext {
		uint64_t unique_mesh_count;
		std::vector<VkFramebuffer> framebuffers;
		Buffer indirect_command_buffer;
		Buffer vertex_buffer;
		Buffer index_buffer;
		VkRenderPass render_pass;
		Pipeline graphics_pipeline;
		VkCommandBuffer command_buffer;
		uint32_t image_write_index;
		VkExtent2D swapchain_extent;
		VkDescriptorSet current_descriptor_set;
		PFN_vkCmdBeginDebugUtilsLabelEXT debug_function_begin;
		PFN_vkCmdEndDebugUtilsLabelEXT debug_function_end;
	};
	void RecordCommandBuffer(const CommandRecordingContext& Context);

	struct Compute_CommandRecordingContext {
		VkCommandBuffer command_buffer;
		Pipeline compute_pipeline;
		VkDescriptorSet current_descriptor_set;
		uint32_t instance_count;
		PFN_vkCmdBeginDebugUtilsLabelEXT debug_function_begin;
		PFN_vkCmdEndDebugUtilsLabelEXT debug_function_end;
	};
	void RecordCommandBuffer(const Compute_CommandRecordingContext& Context);

#pragma endregion

#pragma region Frame Buffers
	// Implemented in "FrameBuffer.cpp"

#pragma endregion

#pragma region Synchronization Primitives

#pragma endregion

#pragma region Graphics and Compute Pipeline
	// Implemented in "Pipelines.cpp"

#pragma endregion

#pragma region Data Buffers
	// Implemented in "DataBuffer.cpp"



#pragma endregion

#pragma region Descriptor Sets
	// Implemented in "DescriptorSet.cpp"

	struct Graphic_DescriptorContext {
		VkDevice logical_device;
		Buffer instance_buffer;
		std::array<BufferMapped, MAX_FRAMES_IN_FLIGHT> uniform_buffers;
		std::array<Buffer, MAX_FRAMES_IN_FLIGHT> should_draw_flags_buffer;
	};
	std::vector<VkDescriptorSet> UpdateDescriptorSets(const Graphic_DescriptorContext& Context, std::vector<VkDescriptorSet> old_set);

	struct Compute_DescriptorContext {
		VkDevice logical_device;
		std::array<Buffer, MAX_FRAMES_IN_FLIGHT> indirect_draw_buffers;
		Buffer instance_centers;
	};
	std::vector<VkDescriptorSet> UpdateComputeUniqueDescriptor(const Compute_DescriptorContext& Context, std::vector<VkDescriptorSet> old_set);

#pragma endregion

#pragma region Asset Loading
	// Implemented in "AssetLoader.cpp"



#pragma endregion

};