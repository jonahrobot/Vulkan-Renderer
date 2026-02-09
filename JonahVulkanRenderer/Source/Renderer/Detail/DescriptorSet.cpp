#include <iostream>
#include <array>
#include "RendererDetail.h"

// Unnamed namespace to show functions below are pure Utility strictly for this .cpp file.
namespace {
}

// Implements all Descriptor Set functions in "RendererDetail.h" to be used in "Renderer.cpp"
namespace renderer::detail {
	
	

	/*
	
		If I could have some struct in my renderer.h that defines what data I want, and what shader I want it
		
		Then we create a descriptor set matching that information?? So it is not hidden inside of the DescriptorSet.cpp

		No hard coding data types would be nice!

		Then we need some like promise system that checks for data buffers creation and updating in descriptors!
		

		
	
	*/

	std::vector<VkDescriptorSet> UpdateDescriptorSets(const Graphic_DescriptorContext& Context, std::vector<VkDescriptorSet> old_set) {

		// Allocate our descriptor sets from the layout template and ubo information
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			VkDescriptorBufferInfo buffer_info{};
			buffer_info.buffer = Context.uniform_buffers[i].buffer.buffer;
			buffer_info.offset = 0;
			buffer_info.range = Context.uniform_buffers[i].buffer.byte_size;

			VkDescriptorBufferInfo instance_buffer_info{};
			instance_buffer_info.buffer = Context.instance_buffer.buffer;
			instance_buffer_info.offset = 0;
			instance_buffer_info.range = Context.instance_buffer.byte_size;

			VkDescriptorBufferInfo should_draw_flags_info{};
			should_draw_flags_info.buffer = Context.should_draw_flags_buffer[i].buffer;
			should_draw_flags_info.offset = 0;
			should_draw_flags_info.range = Context.should_draw_flags_buffer[i].byte_size;

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
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {

			std::array<VkWriteDescriptorSet, 2> descriptor_writes{};

			VkDescriptorBufferInfo indirect_draw_buffer_info{};
			indirect_draw_buffer_info.buffer = Context.indirect_draw_buffers[i].buffer;
			indirect_draw_buffer_info.offset = 0;
			indirect_draw_buffer_info.range = Context.indirect_draw_buffers[i].byte_size;

			descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptor_writes[0].dstSet = old_set[i];
			descriptor_writes[0].dstBinding = 2;
			descriptor_writes[0].dstArrayElement = 0;
			descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			descriptor_writes[0].descriptorCount = 1;
			descriptor_writes[0].pBufferInfo = &indirect_draw_buffer_info;

			VkDescriptorBufferInfo instance_centers_buffer_info{};
			instance_centers_buffer_info.buffer = Context.instance_centers.buffer;
			instance_centers_buffer_info.offset = 0;
			instance_centers_buffer_info.range = Context.instance_centers.byte_size;

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