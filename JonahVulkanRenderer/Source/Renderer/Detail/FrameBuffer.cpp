#include <iostream>
#include "RendererDetail.h"

// Unnamed namespace to show functions below are pure Utility strictly for this .cpp file.
namespace {
}


// Implements the Frame Buffer creation function in "RendererDetail.h" to be used in "Renderer.cpp"
namespace renderer::detail {

	std::vector<VkFramebuffer> CreateFramebuffers(const FrameBufferContext& Context) {

		std::vector<VkFramebuffer> frame_buffers(Context.image_views.size());

		for (size_t i = 0; i < Context.image_views.size(); i++) {
			VkImageView attachments[]{
				Context.image_views[i]
			};

			VkFramebufferCreateInfo framebuffer_info{};
			framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebuffer_info.renderPass = Context.render_pass;
			framebuffer_info.attachmentCount = 1;
			framebuffer_info.pAttachments = attachments;
			framebuffer_info.width = Context.swapchain_extent.width;
			framebuffer_info.height = Context.swapchain_extent.height;
			framebuffer_info.layers = 1;

			if (vkCreateFramebuffer(Context.logical_device, &framebuffer_info, nullptr, &frame_buffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create framebuffer!");
			}
		}

		return frame_buffers;
	}
}