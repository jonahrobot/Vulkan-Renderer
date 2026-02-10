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
#pragma endregion

#pragma region Asset Loading
	// Implemented in "AssetLoader.cpp"



#pragma endregion

};