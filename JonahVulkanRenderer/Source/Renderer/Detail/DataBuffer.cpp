#include <iostream>
#include "RendererDetail.h"

// Unnamed namespace to show functions below are pure Utility strictly for this .cpp file.
namespace {


	/*
	 * Optimization:
	 * Instead of creating a VkBuffer on the GPU and writing to it from the CPU, we create two buffers.
	 * One opens the CPU to write data onto the GPU.
	 * The second is strictly for the GPU.
	 *
	 * The first is a temp buffer, whose data is copied to the GPU only buffer.
	 * This allows us to end with a buffer only on the GPU. Reducing read times as CPU coherency is not required.
	 */



	VkFormat FindSupportedFormat(const std::vector<VkFormat>& Options, VkPhysicalDevice PhysicalDevice, VkImageTiling DesiredTiling, VkFormatFeatureFlags DesiredFeatures) {

		for (VkFormat format : Options) {
			VkFormatProperties properties;
			vkGetPhysicalDeviceFormatProperties(PhysicalDevice, format, &properties);

			bool support_for_linear_tiling = (properties.linearTilingFeatures & DesiredFeatures) == DesiredFeatures;
			bool support_for_optimal_tiling = (properties.optimalTilingFeatures & DesiredFeatures) == DesiredFeatures;

			if (DesiredTiling == VK_IMAGE_TILING_LINEAR && support_for_linear_tiling) {
				return format;
			}
			else if (DesiredTiling == VK_IMAGE_TILING_OPTIMAL && support_for_optimal_tiling) {
				return format;
			}
		}

		throw std::runtime_error("Failed to find a supported format.");
	}

	bool HasStencilComponent(VkFormat Format) {
		return Format == VK_FORMAT_D32_SFLOAT_S8_UINT || Format == VK_FORMAT_D24_UNORM_S8_UINT;
	}
}


// Implements all Vertex Buffer functions in "RendererDetail.h" to be used in "Renderer.cpp"
namespace renderer::detail {


	// Depth formats are ways we can store depth data, along with a possible stencil buffer
	// Only some GPUS support specific formats, so we must check which ours supports.
	VkFormat FindDepthFormat(VkPhysicalDevice PhysicalDevice) {

		std::vector<VkFormat> Options = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };

		return FindSupportedFormat(Options, PhysicalDevice, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}

	GPUResource CreateDepthBuffer(const DepthBufferContext& Context) {

		GPUResource depth_buffer{};

		VkFormat depth_format = FindDepthFormat(Context.physical_device);

		CreateImageContext context_image{};
		context_image.format = depth_format;
		context_image.height = Context.swapchain_extent.height;
		context_image.width = Context.swapchain_extent.width;
		context_image.logical_device = Context.logical_device;
		context_image.physical_device = Context.physical_device;
		context_image.required_properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		context_image.tiling = VK_IMAGE_TILING_OPTIMAL;
		context_image.usage_flags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

		GPUImage created_image = CreateImage(context_image);
		depth_buffer.image = created_image.image;
		depth_buffer.image_memory = created_image.image_memory;

		ImageViewContext context_image_view{};
		context_image_view.image_format = depth_format;
		context_image_view.image = created_image.image;
		context_image_view.logical_device = Context.logical_device;
		context_image_view.aspect_flags = VK_IMAGE_ASPECT_DEPTH_BIT;
		context_image_view.view_type = VK_IMAGE_VIEW_TYPE_2D;

		depth_buffer.image_view = CreateImageView(context_image_view);

		return depth_buffer;
	}

}