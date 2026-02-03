#include <iostream>
#include "RendererDetail.h"

// Unnamed namespace to show functions below are pure Utility strictly for this .cpp file.
namespace {
}


// Implements the Frame Buffer creation function in "RendererDetail.h" to be used in "Renderer.cpp"
namespace renderer::detail {

	std::vector<VkFramebuffer>  CreateFrameBuffers(const VulkanCore& VulkanCore, const SwapchainCore& Swapchain, const VkRenderPass& RenderPass, const DepthBuffer& DepthBuffer){

		std::vector<VkFramebuffer> frame_buffers(Swapchain.swapchain_image_views.size());

		for (size_t i = 0; i < Swapchain.swapchain_image_views.size(); i++) {
			std::array<VkImageView, 2> attachments = {
				Swapchain.swapchain_image_views[i],
				DepthBuffer.image_view
			};

			VkFramebufferCreateInfo framebuffer_info{};
			framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebuffer_info.renderPass = RenderPass;
			framebuffer_info.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebuffer_info.pAttachments = attachments.data();
			framebuffer_info.width = Swapchain.swapchain_extent.width;
			framebuffer_info.height = Swapchain.swapchain_extent.height;
			framebuffer_info.layers = 1;

			if (vkCreateFramebuffer(VulkanCore.logical_device, &framebuffer_info, nullptr, &frame_buffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create framebuffer!");
			}
		}

		return frame_buffers;
	}
}