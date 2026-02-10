#pragma once

#include <vulkan/vulkan.hpp>
#include "VkCommon.h"

namespace renderer::scene {

	class SceneParser {

	public:
		SceneParser(const std::vector<MeshInstances>& NewModelSet);
		std::vector<InstanceData> GetInstanceData();
		std::vector<glm::vec4>	GetModelCenters(); 
		std::vector<VkDrawIndexedIndirectCommand> GetDrawCommands();
		std::vector<Vertex> GetSceneVertices();
		std::vector<uint32_t> GetSceneIndices();
		uint32_t GetMeshCount();

	private:
		std::vector<MeshInstances> model_set;

		std::vector<InstanceData> instance_data;
		std::vector<glm::vec4> model_centers;
		std::vector<VkDrawIndexedIndirectCommand> draw_commands;
		std::vector<Vertex> scene_vertices;
		std::vector<uint32_t> scene_indices;
		uint32_t mesh_count;
	};
}