#include "VkSceneProcesser.h"

namespace renderer::scene {

	SceneParser::SceneParser(const std::vector<MeshInstances>& NewModelSet) {

		model_set = NewModelSet;

		uint32_t m = 0;
		mesh_count = 0;
		scene_vertices = {};
		scene_indices = {};
		draw_commands = {};
		model_centers = {};
		instance_data = {};

		for (MeshInstances model : model_set) {

			Mesh mesh = model.mesh;

			bool no_data = mesh.vertices.size() == 0 || mesh.indices.size() == 0;
			if (no_data) continue;

			mesh_count += model.instance_count;

			// Move vertex data
			uint32_t offset = static_cast<uint32_t>(scene_vertices.size());
			glm::vec3 sum = glm::vec3(0);
			for (const Vertex& v : mesh.vertices) {
				scene_vertices.push_back(v);
				sum += v.position;
			}

			// Move index data
			uint32_t first_index = static_cast<uint32_t>(scene_indices.size());
			for (uint32_t i : mesh.indices) {
				scene_indices.push_back(i + offset);
			}

			// Create draw command
			VkDrawIndexedIndirectCommand indirect_command{};
			indirect_command.instanceCount = model.instance_count;
			indirect_command.firstInstance = m;
			indirect_command.firstIndex = first_index;
			indirect_command.indexCount = static_cast<uint32_t>(mesh.indices.size());

			draw_commands.push_back(indirect_command);

			m += model.instance_count;

			float length = mesh.vertices.size();
			glm::vec4 mesh_center_point = glm::vec4(sum.x / length, sum.y / length, sum.z / length, 1);

			// Find model center and instance data
			for (int i = 0; i < model.instance_count; i++){
				
				const glm::mat4& instance_model_matrix = model.instance_model_matrices[i];

				glm::vec4 center_with_offset = instance_model_matrix[3] + mesh_center_point;
				center_with_offset.w = 1;
				model_centers.push_back(center_with_offset);
				instance_data.push_back({ instance_model_matrix , glm::vec4(0)});
			}
		}
	}

	std::vector<InstanceData> SceneParser::GetInstanceData() {
		return instance_data;
	}

	std::vector<glm::vec4> SceneParser::GetModelCenters(){
		return model_centers;
	}

	std::vector<VkDrawIndexedIndirectCommand> SceneParser::GetDrawCommands() {
		return draw_commands;
	}

	std::vector<Vertex> SceneParser::GetSceneVertices() {
		return scene_vertices;
	}

	std::vector<uint32_t> SceneParser::GetSceneIndices() {
		return scene_indices;
	}

	uint32_t SceneParser::GetMeshCount() {
		return mesh_count;
	}
}