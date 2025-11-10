#include <iostream>
#include "RendererDetail.h"

// Unnamed namespace to show functions below are pure Utility strictly for this .cpp file.
namespace {
}


// Implements all Descriptor Set functions in "RendererDetail.h" to be used in "Renderer.cpp"
namespace renderer::detail {

	VkDescriptorSetLayout CreateDescriptorLayout(const DescriptorLayoutContext& Context) {

		VkDescriptorSetLayout new_layout;

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

		if (vkCreateDescriptorSetLayout(Context.logical_device, &layout_info, nullptr, &new_layout) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create descriptor set layout.");
		}

		return new_layout;
	}

	VkDescriptorPool CreateDescriptorPool(const DescriptorPoolContext& Context) {

		VkDescriptorPool new_pool;

		VkDescriptorPoolSize pool_size{};
		pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		pool_size.descriptorCount = static_cast<uint32_t>(Context.max_frames_in_flight);

		VkDescriptorPoolCreateInfo pool_info{};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.poolSizeCount = 1;
		pool_info.pPoolSizes = &pool_size;
		pool_info.maxSets = static_cast<uint32_t>(Context.max_frames_in_flight);

		if (vkCreateDescriptorPool(Context.logical_device, &pool_info, nullptr, &new_pool) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create descriptor pool.");
		}

		return new_pool;
	}

	std::vector<VkDescriptorSet> CreateDescriptorSets(const DescriptorSetContext& Context) {

		std::vector<VkDescriptorSet> descriptor_sets(Context.max_frames_in_flight);

		// Create descriptor sets
		std::vector<VkDescriptorSetLayout> layouts(Context.max_frames_in_flight, Context.descriptor_set_layout);
		
		VkDescriptorSetAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		alloc_info.descriptorPool = Context.descriptor_pool;
		alloc_info.descriptorSetCount = static_cast<uint32_t>(Context.max_frames_in_flight);
		alloc_info.pSetLayouts = layouts.data();

		if (vkAllocateDescriptorSets(Context.logical_device, &alloc_info, descriptor_sets.data()) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate descriptor set.");
		}
		
		// Allocate our descriptor sets from the layout template and ubo information
		for (size_t i = 0; i < Context.max_frames_in_flight; i++) {
			VkDescriptorBufferInfo buffer_info{};
			buffer_info.buffer = Context.uniform_buffers[i];
			buffer_info.offset = 0;
			buffer_info.range = Context.ubo_size;

			VkWriteDescriptorSet descriptor_write{};
			descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptor_write.dstSet = descriptor_sets[i];
			descriptor_write.dstBinding = 0;
			descriptor_write.dstArrayElement = 0;
			descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptor_write.descriptorCount = 1;
			descriptor_write.pBufferInfo = &buffer_info;
			descriptor_write.pImageInfo = nullptr;
			descriptor_write.pTexelBufferView = nullptr;

			vkUpdateDescriptorSets(Context.logical_device, 1, &descriptor_write, 0, nullptr);
		}

		return descriptor_sets;
	}

}