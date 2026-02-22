#pragma once

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL 
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>
#include "VkCommon.h"

namespace renderer::scene {

	class SceneParser {

	public:
		SceneParser(const std::vector<MeshInstances>& NewModelSet);
		std::vector<InstanceData> GetInstanceData();
		std::vector<BoundingBoxData> GetBoundingData();
		std::vector<VkDrawIndexedIndirectCommand> GetDrawCommands();
		std::vector<Vertex> GetSceneVertices();
		std::vector<uint32_t> GetSceneIndices();
		uint32_t GetMeshCount();
		glm::vec3 GetSceneRoot();

	private:
		std::vector<MeshInstances> model_set;

		std::vector<InstanceData> instance_data;
		std::vector<BoundingBoxData> bounding_data;
		std::vector<VkDrawIndexedIndirectCommand> draw_commands;
		std::vector<Vertex> scene_vertices;
		std::vector<uint32_t> scene_indices;
		uint32_t mesh_count;
		glm::vec3 scene_root;
	};
}