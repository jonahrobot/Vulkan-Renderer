#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_CXX2A
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <array>

#define MAX_FRAMES_IN_FLIGHT 2

namespace renderer::detail {

#pragma region Uniform and Instance Data

	struct InstanceData {
		alignas(16) glm::mat4 model;
		alignas(16) glm::vec4 array_index;
	};

	struct UniformBufferObject {
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
		alignas(16) glm::vec4 frustum_planes[6];
	};

#pragma endregion

#pragma region Vertex Type
	

	struct MergedIndexVertexBuffer {
		std::vector<Vertex> merged_vertex_buffer;
		std::vector<uint32_t> merged_index_buffer;
	};

	MergedIndexVertexBuffer MergeIndexVertexBuffer(const std::vector<Vertex>& V1, const std::vector<uint32_t>& I1, const std::vector<Vertex>& V2, const std::vector<uint32_t>& I2);
#pragma endregion

#pragma region Command Buffer Recording


#pragma endregion

#pragma region Image Creation
	struct GPUResource {
		VkImage image;
		VkDeviceMemory image_memory;
		VkImageView image_view;
	};

	struct CreateImageContext {
		VkDevice logical_device;
		VkPhysicalDevice physical_device;
		uint32_t width;
		uint32_t height;
		VkFormat format;
		VkImageTiling tiling;
		VkImageUsageFlags usage_flags;
		VkMemoryPropertyFlags required_properties;
		uint32_t array_layers = 1;
	};
	struct GPUImage {
		VkImage image;
		VkDeviceMemory image_memory;
	};
	GPUImage CreateImage(const CreateImageContext& Context);

	struct ImageViewContext {
		VkDevice logical_device;
		VkImage image;
		VkFormat image_format;
		VkImageAspectFlags aspect_flags;
		VkImageViewType view_type;
		uint32_t array_layers = 1;
	};
	VkImageView CreateImageView(const ImageViewContext& Context);
	uint32_t FindMemoryType(VkPhysicalDevice PhysicalDevice, uint32_t TypeFilter, VkMemoryPropertyFlags properties);
#pragma endregion

}

// Define Vertex hash function
namespace std {
	template<> struct hash<renderer::detail::Vertex> {
		size_t operator()(renderer::detail::Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.position) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1);
		}
	};
}