#include <iostream>
#include "RendererDetail.h"

// Unnamed namespace to show functions below are pure Utility strictly for this .cpp file.
namespace {

	renderer::detail::Buffer CreateDataBuffer(VkDevice LogicalDevice, VkPhysicalDevice PhysicalDevice, VkDeviceSize BufferSize, VkBufferUsageFlags UsageFlags, VkMemoryPropertyFlags PropertyFlags) {

		VkBuffer buffer;

		if (BufferSize == 0) {
			throw std::runtime_error("Failed to create Data Buffer, no data provided.");
		}

		// Create buffer
		VkBufferCreateInfo buffer_info{};
		buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_info.size = BufferSize;
		buffer_info.usage = UsageFlags;
		buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(LogicalDevice, &buffer_info, nullptr, &buffer) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create vertex buffer.");
		}

		// Allocate buffer memory
		VkDeviceMemory buffer_memory;

		VkMemoryRequirements memory_requirements;
		vkGetBufferMemoryRequirements(LogicalDevice, buffer, &memory_requirements);

		VkMemoryAllocateInfo allocate_info{};
		allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocate_info.allocationSize = memory_requirements.size;
		allocate_info.memoryTypeIndex = renderer::detail::FindMemoryType(PhysicalDevice, memory_requirements.memoryTypeBits, PropertyFlags);

		if (vkAllocateMemory(LogicalDevice, &allocate_info, nullptr, &buffer_memory) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate vertex buffer memory.");
		}

		vkBindBufferMemory(LogicalDevice, buffer, buffer_memory, 0);

		renderer::detail::Buffer return_data{};
		return_data.buffer = buffer;
		return_data.memory = buffer_memory;
		return_data.byte_size = BufferSize;

		return return_data;
	}

	// Copy two buffers on the GPU through GPU commands.
	void CopyBuffer(VkQueue GraphicsQueue, VkDevice LogicalDevice, VkCommandPool CommandPool, VkBuffer SrcBuffer, VkBuffer DstBuffer, VkDeviceSize size) {
		
		VkCommandBuffer command_buffer = renderer::detail::BeginSingleTimeCommand(CommandPool, LogicalDevice);

		VkBufferCopy copy_region{};
		copy_region.srcOffset = 0;
		copy_region.dstOffset = 0;
		copy_region.size = size;
		vkCmdCopyBuffer(command_buffer, SrcBuffer, DstBuffer, 1, &copy_region);

		renderer::detail::EndSingleTimeCommand(command_buffer, CommandPool, LogicalDevice, GraphicsQueue);
	}

	/*
	 * Optimization:
	 * Instead of creating a VkBuffer on the GPU and writing to it from the CPU, we create two buffers.
	 * One opens the CPU to write data onto the GPU.
	 * The second is strictly for the GPU.
	 *
	 * The first is a temp buffer, whose data is copied to the GPU only buffer.
	 * This allows us to end with a buffer only on the GPU. Reducing read times as CPU coherency is not required.
	 */

	renderer::detail::Buffer CreateGPULocalBuffer(const void* DataSrc, VkDeviceSize DataSize, VkBufferUsageFlags UsageFlags, VkDevice LogicalDevice, VkPhysicalDevice PhysicalDevice, VkQueue GraphicsQueue, VkCommandPool CommandPool) {

		if (DataSize == 0) {
			throw std::runtime_error("Failed to create GPU local buffer, no data provided.");
		}

		// Create temp buffer that CPU and GPU can both see and write to.
		VkBufferUsageFlags temp_usage_flags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		VkMemoryPropertyFlags temp_property_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		renderer::detail::Buffer temp_buffer_data = CreateDataBuffer(LogicalDevice, PhysicalDevice, DataSize, temp_usage_flags, temp_property_flags);
		VkBuffer temp_buffer = temp_buffer_data.buffer;
		VkDeviceMemory temp_buffer_memory = temp_buffer_data.memory;

		void* data;
		vkMapMemory(LogicalDevice, temp_buffer_memory, 0, DataSize, 0, &data);
		memcpy(data, DataSrc, (size_t)DataSize);
		vkUnmapMemory(LogicalDevice, temp_buffer_memory);

		// Create GPU only buffer that CPU can't see
		VkBufferUsageFlags usage_flags = UsageFlags;
		VkMemoryPropertyFlags property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

		renderer::detail::Buffer created_buffer_data = CreateDataBuffer(LogicalDevice, PhysicalDevice, DataSize, usage_flags, property_flags);

		// Copy temp buffer data into GPU only buffer
		CopyBuffer(GraphicsQueue, LogicalDevice, CommandPool, temp_buffer, created_buffer_data.buffer, DataSize);

		// Destroy temp
		vkDestroyBuffer(LogicalDevice, temp_buffer, nullptr);
		vkFreeMemory(LogicalDevice, temp_buffer_memory, nullptr);

		return created_buffer_data;
	}

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
	
	template<>
	Buffer CreateLocalBuffer<Vertex>(const BufferContext& Context, const std::vector<Vertex>& Data, VkBufferUsageFlags UsageFlags) {

		const void* data_src = Data.data();
		VkDeviceSize data_size = sizeof(Data[0]) * Data.size();

		return CreateGPULocalBuffer(data_src, data_size, UsageFlags, Context.logical_device,
				Context.physical_device, Context.graphics_queue, Context.command_pool);
	}

	template<>
	Buffer CreateLocalBuffer<uint32_t>(const BufferContext& Context, const std::vector<uint32_t>& Data, VkBufferUsageFlags UsageFlags) {

		const void* data_src = Data.data();
		VkDeviceSize data_size = sizeof(Data[0]) * Data.size();

		return CreateGPULocalBuffer(data_src, data_size, UsageFlags, Context.logical_device,
				Context.physical_device, Context.graphics_queue, Context.command_pool);
	}

	template<>
	Buffer CreateLocalBuffer<VkDrawIndexedIndirectCommand>(const BufferContext& Context, const std::vector<VkDrawIndexedIndirectCommand>& Data, VkBufferUsageFlags UsageFlags) {

		const void* data_src = Data.data();
		VkDeviceSize data_size = sizeof(Data[0]) * Data.size();

		return CreateGPULocalBuffer(data_src, data_size, UsageFlags, Context.logical_device,
				Context.physical_device, Context.graphics_queue, Context.command_pool);
	}

	template<>
	Buffer CreateLocalBuffer<InstanceData>(const BufferContext& Context, const std::vector<InstanceData>& Data, VkBufferUsageFlags UsageFlags) {

		const void* data_src = Data.data();
		VkDeviceSize data_size = sizeof(Data[0]) * Data.size();

		return CreateGPULocalBuffer(data_src, data_size, UsageFlags, Context.logical_device,
			Context.physical_device, Context.graphics_queue, Context.command_pool);
	}

	template<>
	Buffer CreateLocalBuffer<glm::vec4>(const BufferContext& Context, const std::vector<glm::vec4>& Data, VkBufferUsageFlags UsageFlags) {

		const void* data_src = Data.data();
		VkDeviceSize data_size = sizeof(Data[0]) * Data.size();

		return CreateGPULocalBuffer(data_src, data_size, UsageFlags, Context.logical_device,
			Context.physical_device, Context.graphics_queue, Context.command_pool);
	}

	void DestroyBuffer(VkDevice LogicalDevice, BufferMapped& Buffer) {
		DestroyBuffer(LogicalDevice, Buffer.buffer);
	}

	void DestroyBuffer(VkDevice LogicalDevice, Buffer& Buffer) {
		if (Buffer.buffer != NULL) {
			vkDestroyBuffer(LogicalDevice, Buffer.buffer, nullptr);
			vkFreeMemory(LogicalDevice, Buffer.memory, nullptr);
			Buffer.buffer = NULL;
			Buffer.memory = NULL;
		}
	}

	BufferMapped CreateMappedBuffer(const MappedBufferContext& Context) {

		BufferMapped buffer_wrapper;

		VkBufferUsageFlags usage_flags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		VkMemoryPropertyFlags property_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		buffer_wrapper.buffer = CreateDataBuffer(Context.logical_device, Context.physical_device, Context.buffer_size, usage_flags, property_flags);

		vkMapMemory(Context.logical_device, buffer_wrapper.buffer.memory, 0, Context.buffer_size, 0, &buffer_wrapper.buffer_mapped);
		
		return buffer_wrapper;
	}

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