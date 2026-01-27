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
	};

#pragma endregion

#pragma region Vertex Type
	struct Vertex {
		glm::vec3 position;
		glm::vec3 color;
		glm::vec2 tex_coord;

		static VkVertexInputBindingDescription GetBindingDescription() {
			VkVertexInputBindingDescription binding_description{};
			binding_description.binding = 0;
			binding_description.stride = sizeof(Vertex);
			binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			return binding_description;
		}

		static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescription() {
			std::array<VkVertexInputAttributeDescription, 3> attribute_descriptions = {};

			// Position
			attribute_descriptions[0].binding = 0;
			attribute_descriptions[0].location = 0;
			attribute_descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT; // Vec3
			attribute_descriptions[0].offset = offsetof(Vertex, position);

			// Color
			attribute_descriptions[1].binding = 0;
			attribute_descriptions[1].location = 1;
			attribute_descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT; // Vec3
			attribute_descriptions[1].offset = offsetof(Vertex, color);

			// Texture Coordinates
			attribute_descriptions[2].binding = 0;
			attribute_descriptions[2].location = 2;
			attribute_descriptions[2].format = VK_FORMAT_R32G32_SFLOAT; // Vec2
			attribute_descriptions[2].offset = offsetof(Vertex, tex_coord);

			return attribute_descriptions;
		}

		bool operator==(const Vertex& other) const {
			return position == other.position && color == other.color && tex_coord == other.tex_coord;
		}
	};

	struct MergedIndexVertexBuffer {
		std::vector<Vertex> merged_vertex_buffer;
		std::vector<uint32_t> merged_index_buffer;
	};

	MergedIndexVertexBuffer MergeIndexVertexBuffer(const std::vector<Vertex>& V1, const std::vector<uint32_t>& I1, const std::vector<Vertex>& V2, const std::vector<uint32_t>& I2);
#pragma endregion


#pragma region Buffer Creation
	struct BufferData {
		VkBuffer created_buffer;
		VkDeviceMemory memory_allocated_for_buffer;
		
		enum BufferCreationErrors{SUCCESS, SIZEZERO};
		BufferCreationErrors err_code;
	};
	BufferData CreateDataBuffer(VkDevice LogicalDevice, VkPhysicalDevice PhysicalDevice, VkDeviceSize BufferSize, VkBufferUsageFlags UsageFlags, VkMemoryPropertyFlags PropertyFlags);

	uint32_t FindMemoryType(VkPhysicalDevice PhysicalDevice, uint32_t TypeFilter, VkMemoryPropertyFlags properties);

#pragma endregion

#pragma region Command Buffer Recording

	VkCommandBuffer BeginSingleTimeCommand(VkCommandPool CommandPool, VkDevice LogicalDevice);

	void EndSingleTimeCommand(VkCommandBuffer CommandBuffer, VkCommandPool CommandPool, VkDevice LogicalDevice, VkQueue GraphicsQueue);

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
#pragma endregion

}

// Define Vertex hash function
namespace std {
	template<> struct hash<renderer::detail::Vertex> {
		size_t operator()(renderer::detail::Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.position) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.tex_coord) << 1);
		}
	};
}