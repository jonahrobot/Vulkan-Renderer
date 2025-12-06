#include <iostream>
#include "RendererDetail.h"

// Unnamed namespace to show functions below are pure Utility strictly for this .cpp file.
namespace {
}


// Implements all Vulkan Command Buffer functions in "RendererDetail.h" to be used in "Renderer.cpp"
namespace renderer::detail {

	VkCommandPool CreateCommandPool(const VkDevice LogicalDevice, uint32_t GraphicsFamilyIndex) {

		VkCommandPoolCreateInfo pool_info{};
		pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		pool_info.queueFamilyIndex = GraphicsFamilyIndex;

		VkCommandPool out_pool;
		if (vkCreateCommandPool(LogicalDevice, &pool_info, nullptr, &out_pool) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create command pool.");
		}
		return out_pool;
	}

	std::vector<VkCommandBuffer> CreateCommandBuffers(const int TotalFrames, const VkDevice LogicalDevice, const VkCommandPool CommandPool) {

		std::vector<VkCommandBuffer> out_buffers;
		out_buffers.resize(TotalFrames);

		VkCommandBufferAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.commandPool = CommandPool;
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandBufferCount = (uint32_t)TotalFrames;

		if (vkAllocateCommandBuffers(LogicalDevice, &alloc_info, out_buffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate command buffers.");
		}
		return out_buffers;
	}

	void RecordCommandBuffer(const CommandRecordingContext& Context) {
		VkCommandBufferBeginInfo begin_info{};
		begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		begin_info.flags = 0;
		begin_info.pInheritanceInfo = nullptr;

		if (vkBeginCommandBuffer(Context.command_buffer, &begin_info) != VK_SUCCESS) {
			throw std::runtime_error("Failed to begin recording command buffer.");
		}

		VkRenderPassBeginInfo render_pass_info{};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		render_pass_info.renderPass = Context.render_pass;
		render_pass_info.framebuffer = Context.framebuffers[Context.image_write_index];
		render_pass_info.renderArea.offset = { 0,0 };
		render_pass_info.renderArea.extent = Context.swapchain_extent;

		std::array<VkClearValue, 2> clear_values;
		clear_values[0].color = {{0.0f,0.0f,0.0f,1.0f}};
		clear_values[1].depthStencil = { 1.0f,0 };

		render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
		render_pass_info.pClearValues = clear_values.data();

		vkCmdBeginRenderPass(Context.command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(Context.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, Context.graphics_pipeline);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(Context.swapchain_extent.width);
		viewport.height = static_cast<float>(Context.swapchain_extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(Context.command_buffer, 0, 1, &viewport);

		VkRect2D scissor{};
		scissor.offset = { 0,0 };
		scissor.extent = Context.swapchain_extent;
		vkCmdSetScissor(Context.command_buffer, 0, 1, &scissor);

		if (Context.total_vertices != 0) {
			VkBuffer vertex_buffers[] = { Context.vertex_buffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(Context.command_buffer, 0, 1, vertex_buffers, offsets);

			vkCmdBindIndexBuffer(Context.command_buffer, Context.index_buffer, 0, VK_INDEX_TYPE_UINT32);

			vkCmdBindDescriptorSets(Context.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
				Context.graphics_pipeline_layout, 0, 1, &Context.current_descriptor_set, 0, nullptr);

			uint32_t size_of_command = sizeof(VkDrawIndexedIndirectCommand);

			for (uint32_t x = 0; x < Context.total_meshes; x++) {
				vkCmdDrawIndexedIndirect(Context.command_buffer, Context.indirect_command_buffer, x * size_of_command, 1, size_of_command);
			}
		}

		vkCmdEndRenderPass(Context.command_buffer);
		if (vkEndCommandBuffer(Context.command_buffer) != VK_SUCCESS) {
			throw std::runtime_error("Failed to record command buffer.");
		}
	}
} // namespace renderer::detail