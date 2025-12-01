#include <iostream>
#include <array>
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

		VkDescriptorSetLayoutBinding sampler_layout_binding{};
		sampler_layout_binding.binding = 1;
		sampler_layout_binding.descriptorCount = 1;
		sampler_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		sampler_layout_binding.pImmutableSamplers = nullptr;
		sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		std::array<VkDescriptorSetLayoutBinding, 2> bindings = { ubo_layout_binding, sampler_layout_binding };

		VkDescriptorSetLayoutCreateInfo layout_info{};
		layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
		layout_info.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(Context.logical_device, &layout_info, nullptr, &new_layout) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create descriptor set layout.");
		}

		return new_layout;
	}

	VkDescriptorPool CreateDescriptorPool(const DescriptorPoolContext& Context) {

		VkDescriptorPool new_pool;

		std::array<VkDescriptorPoolSize,2> pool_sizes{};

		pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		pool_sizes[0].descriptorCount = static_cast<uint32_t>(Context.max_frames_in_flight);

		pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		pool_sizes[1].descriptorCount = static_cast<uint32_t>(Context.max_frames_in_flight);

		VkDescriptorPoolCreateInfo pool_info{};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
		pool_info.pPoolSizes = pool_sizes.data();
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

			VkDescriptorImageInfo image_info{};
			image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			image_info.imageView = Context.image_view;
			image_info.sampler = Context.texture_sampler;

			std::array<VkWriteDescriptorSet, 2> descriptor_writes{};

			descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptor_writes[0].dstSet = descriptor_sets[i];
			descriptor_writes[0].dstBinding = 0;
			descriptor_writes[0].dstArrayElement = 0;
			descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptor_writes[0].descriptorCount = 1;
			descriptor_writes[0].pBufferInfo = &buffer_info;

			descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptor_writes[1].dstSet = descriptor_sets[i];
			descriptor_writes[1].dstBinding = 1;
			descriptor_writes[1].dstArrayElement = 0;
			descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptor_writes[1].descriptorCount = 1;
			descriptor_writes[1].pImageInfo = &image_info;

			vkUpdateDescriptorSets(Context.logical_device, static_cast<uint32_t>(descriptor_writes.size()), descriptor_writes.data(), 0, nullptr);
		}

		return descriptor_sets;
	}

	std::vector<VkDescriptorSet> UpdateDescriptorSets(const DescriptorSetContext& Context, std::vector<VkDescriptorSet> old_set) {

		// Allocate our descriptor sets from the layout template and ubo information
		for (size_t i = 0; i < Context.max_frames_in_flight; i++) {
			VkDescriptorBufferInfo buffer_info{};
			buffer_info.buffer = Context.uniform_buffers[i];
			buffer_info.offset = 0;
			buffer_info.range = Context.ubo_size;

			VkDescriptorImageInfo image_info{};
			image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			image_info.imageView = Context.image_view;
			image_info.sampler = Context.texture_sampler;

			std::array<VkWriteDescriptorSet, 2> descriptor_writes{};

			descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptor_writes[0].dstSet = old_set[i];
			descriptor_writes[0].dstBinding = 0;
			descriptor_writes[0].dstArrayElement = 0;
			descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptor_writes[0].descriptorCount = 1;
			descriptor_writes[0].pBufferInfo = &buffer_info;

			descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptor_writes[1].dstSet = old_set[i];
			descriptor_writes[1].dstBinding = 1;
			descriptor_writes[1].dstArrayElement = 0;
			descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptor_writes[1].descriptorCount = 1;
			descriptor_writes[1].pImageInfo = &image_info;

			vkUpdateDescriptorSets(Context.logical_device, static_cast<uint32_t>(descriptor_writes.size()), descriptor_writes.data(), 0, nullptr);
		}

		return old_set;
	}

}