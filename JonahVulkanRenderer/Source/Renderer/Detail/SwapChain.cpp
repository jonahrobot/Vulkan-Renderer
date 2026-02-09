#include "RendererDetail.h"
#include <algorithm>
#include <iostream>

// Unnamed namespace to show functions below are pure Utility strictly for this .cpp file.
namespace {

}

// Implements all Vulkan SwapChain Creation functions in "RendererDetail.h" to be used in "Renderer.cpp"
namespace renderer::detail {

	RecreateSwapchainData RecreateSwapchain(RecreateSwapchainContext Context) {

		RecreateSwapchainData out_data = {};

		VkDevice logical_device = Context.swapchain_creation_data.logical_device;

		// If window minimized, pause execution.
		int width = 0, height = 0;
		glfwGetFramebufferSize(Context.swapchain_creation_data.window, &width, &height);
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(Context.swapchain_creation_data.window, &width, &height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(logical_device);

		// Cleanup old swapchain data

		vkDestroyImageView(logical_device, Context.OLD_depth_buffer.image_view, nullptr);
		vkDestroyImage(logical_device, Context.OLD_depth_buffer.image, nullptr);
		vkFreeMemory(logical_device, Context.OLD_depth_buffer.image_memory, nullptr);

		for (auto framebuffer : Context.OLD_framebuffers) {
			vkDestroyFramebuffer(logical_device, framebuffer, nullptr);
		}
		for (auto view : Context.OLD_swapchain_image_views) {
			vkDestroyImageView(logical_device, view, nullptr);
		}
		vkDestroySwapchainKHR(logical_device, Context.OLD_swapchain, nullptr);

		// Create new swapchain data

		out_data.swapchain_data = detail::CreateSwapchain(Context.swapchain_creation_data);

		out_data.swapchain_images = detail::GetSwapchainImages(out_data.swapchain_data.swapchain, logical_device);

		out_data.swapchain_image_views = detail::CreateSwapchainViews(out_data.swapchain_images, logical_device, out_data.swapchain_data.swapchain_image_format);

		// Create Depth Resource
		detail::DepthBufferContext context_depth_buffer = {};
		context_depth_buffer.logical_device = logical_device;
		context_depth_buffer.physical_device = Context.swapchain_creation_data.physical_device;
		context_depth_buffer.swapchain_extent = out_data.swapchain_data.swapchain_extent;

		out_data.depth_buffer = CreateDepthBuffer(context_depth_buffer);

		detail::FrameBufferContext context_framebuffer = {};
		context_framebuffer.image_views = out_data.swapchain_image_views;
		context_framebuffer.render_pass = Context.render_pass;
		context_framebuffer.swapchain_extent = out_data.swapchain_data.swapchain_extent;
		context_framebuffer.logical_device = logical_device;
		context_framebuffer.depth_image_view = out_data.depth_buffer.image_view;

		out_data.framebuffers = detail::CreateFramebuffers(context_framebuffer);

		return out_data;
	}

} // namespace renderer::detail