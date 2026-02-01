#include <iostream>
#include <array>
#include "RendererDetail.h"

// Unnamed namespace to show functions below are pure Utility strictly for this .cpp file.
namespace {
}

// Implements all Descriptor Set functions in "RendererDetail.h" to be used in "Renderer.cpp"
namespace renderer::detail {
	
	VkDescriptorPool CreateDescriptorPool(const VkDevice& LogicalDevice, uint8_t MaxFramesInFlight) {

		VkDescriptorPool new_pool;

		std::array<VkDescriptorPoolSize, 2> pool_sizes{};

		pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		pool_sizes[0].descriptorCount = static_cast<uint32_t>(MaxFramesInFlight);

		pool_sizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		pool_sizes[1].descriptorCount = static_cast<uint32_t>(MaxFramesInFlight) * 4;

		VkDescriptorPoolCreateInfo pool_info{};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
		pool_info.pPoolSizes = pool_sizes.data();
		pool_info.maxSets = static_cast<uint32_t>(MaxFramesInFlight);

		if (vkCreateDescriptorPool(LogicalDevice, &pool_info, nullptr, &new_pool) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create descriptor pool.");
		}

		return new_pool;
	}

	VkDescriptorSetLayout CreateDescriptorLayout(const VkDevice& LogicalDevice) {

		VkDescriptorSetLayout new_layout;

		VkDescriptorSetLayoutBinding ubo_layout_binding{};
		ubo_layout_binding.binding = 0;
		ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		ubo_layout_binding.descriptorCount = 1;
		ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
		ubo_layout_binding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding instance_data_layout_binding{};
		instance_data_layout_binding.binding = 1;
		instance_data_layout_binding.descriptorCount = 1;
		instance_data_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		instance_data_layout_binding.pImmutableSamplers = nullptr;
		instance_data_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutBinding compute_draws_binding{};
		compute_draws_binding.binding = 2;
		compute_draws_binding.descriptorCount = 1;
		compute_draws_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		compute_draws_binding.pImmutableSamplers = nullptr;
		compute_draws_binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

		VkDescriptorSetLayoutBinding instance_centers_layout_binding{};
		instance_centers_layout_binding.binding = 3;
		instance_centers_layout_binding.descriptorCount = 1;
		instance_centers_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		instance_centers_layout_binding.pImmutableSamplers = nullptr;
		instance_centers_layout_binding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

		VkDescriptorSetLayoutBinding should_draw_flags_binding{};
		should_draw_flags_binding.binding = 4;
		should_draw_flags_binding.descriptorCount = 1;
		should_draw_flags_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		should_draw_flags_binding.pImmutableSamplers = nullptr;
		should_draw_flags_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT;

		std::array<VkDescriptorSetLayoutBinding, 5> bindings = { ubo_layout_binding, instance_data_layout_binding, compute_draws_binding, instance_centers_layout_binding, should_draw_flags_binding };

		VkDescriptorSetLayoutCreateInfo layout_info{};
		layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layout_info.bindingCount = static_cast<uint32_t>(bindings.size());
		layout_info.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(LogicalDevice, &layout_info, nullptr, &new_layout) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create descriptor set layout.");
		}

		return new_layout;
	}

	std::vector<VkDescriptorSet> CreateDescriptorSets(const DescriptorCreateContext& Context) {

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

		return descriptor_sets;
	}

	std::vector<VkDescriptorSet> UpdateDescriptorSets(const Graphic_DescriptorContext& Context, std::vector<VkDescriptorSet> old_set) {

		// Allocate our descriptor sets from the layout template and ubo information
		for (size_t i = 0; i < Context.max_frames_in_flight; i++) {
			VkDescriptorBufferInfo buffer_info{};
			buffer_info.buffer = Context.uniform_buffers[i];
			buffer_info.offset = 0;
			buffer_info.range = Context.ubo_size;

			VkDescriptorBufferInfo instance_buffer_info{};
			instance_buffer_info.buffer = Context.instance_buffer;
			instance_buffer_info.offset = 0;
			instance_buffer_info.range = Context.instance_buffer_size;

			VkDescriptorBufferInfo should_draw_flags_info{};
			should_draw_flags_info.buffer = Context.should_draw_flags_buffer[i];
			should_draw_flags_info.offset = 0;
			should_draw_flags_info.range = Context.should_draw_flags_buffer_size;

			std::array<VkWriteDescriptorSet, 3> descriptor_writes{};

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
			descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			descriptor_writes[1].descriptorCount = 1;
			descriptor_writes[1].pBufferInfo = &instance_buffer_info;

			descriptor_writes[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptor_writes[2].dstSet = old_set[i];
			descriptor_writes[2].dstBinding = 4;
			descriptor_writes[2].dstArrayElement = 0;
			descriptor_writes[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			descriptor_writes[2].descriptorCount = 1;
			descriptor_writes[2].pBufferInfo = &should_draw_flags_info;

			vkUpdateDescriptorSets(Context.logical_device, static_cast<uint32_t>(descriptor_writes.size()), descriptor_writes.data(), 0, nullptr);
		}

		return old_set;
	}

	std::vector<VkDescriptorSet> UpdateComputeUniqueDescriptor(const Compute_DescriptorContext& Context, std::vector<VkDescriptorSet> old_set) {

		// Allocate our descriptor sets from the layout template and ubo information
		for (size_t i = 0; i < Context.max_frames_in_flight; i++) {

			std::array<VkWriteDescriptorSet, 2> descriptor_writes{};

			VkDescriptorBufferInfo indirect_draw_buffer_info{};
			indirect_draw_buffer_info.buffer = Context.indirect_draw_buffers[i];
			indirect_draw_buffer_info.offset = 0;
			indirect_draw_buffer_info.range = Context.indirect_draw_buffer_size;

			descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptor_writes[0].dstSet = old_set[i];
			descriptor_writes[0].dstBinding = 2;
			descriptor_writes[0].dstArrayElement = 0;
			descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			descriptor_writes[0].descriptorCount = 1;
			descriptor_writes[0].pBufferInfo = &indirect_draw_buffer_info;

			VkDescriptorBufferInfo instance_centers_buffer_info{};
			instance_centers_buffer_info.buffer = Context.instance_centers_buffer;
			instance_centers_buffer_info.offset = 0;
			instance_centers_buffer_info.range = Context.instance_centers_buffer_size;

			descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptor_writes[1].dstSet = old_set[i];
			descriptor_writes[1].dstBinding = 3;
			descriptor_writes[1].dstArrayElement = 0;
			descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			descriptor_writes[1].descriptorCount = 1;
			descriptor_writes[1].pBufferInfo = &instance_centers_buffer_info;

			vkUpdateDescriptorSets(Context.logical_device, static_cast<uint32_t>(descriptor_writes.size()), descriptor_writes.data(), 0, nullptr);
		}

		return old_set;
	}

}