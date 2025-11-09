#include <iostream>
#include "RendererDetail.h"

// Unnamed namespace to show functions below are pure Utility strictly for this .cpp file.
namespace {
}


// Implements all Descriptor Set functions in "RendererDetail.h" to be used in "Renderer.cpp"
namespace renderer::detail {

	VkDescriptorSetLayout CreateDescriptorLayout(const DescriptorLayoutContext& Context) {

		VkDescriptorSetLayoutBinding ubo_layout_binding{};
		ubo_layout_binding.binding = 0;
		ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		ubo_layout_binding.descriptorCount = 1;
		ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		ubo_layout_binding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo layout_info{};
		layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layout_info.bindingCount = 1;
		layout_info.pBindings = &ubo_layout_binding;

		VkDescriptorSetLayout new_layout;
		if (vkCreateDescriptorSetLayout(Context.logical_device, &layout_info, nullptr, &new_layout) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create descriptor set layout.");
		}
	}

}