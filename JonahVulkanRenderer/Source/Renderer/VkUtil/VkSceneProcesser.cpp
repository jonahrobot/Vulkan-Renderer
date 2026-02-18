#include "VkSceneProcesser.h"

namespace renderer::scene {

	SceneParser::SceneParser(const std::vector<MeshInstances>& NewModelSet) {

		model_set = NewModelSet;

		uint32_t m = 0;
		mesh_count = 0;
		scene_vertices = {};
		scene_indices = {};
		draw_commands = {};
		bounding_data = {};
		instance_data = {};

		for (MeshInstances model : model_set) {

			Mesh mesh = model.mesh;

			bool no_data = mesh.vertices.size() == 0 || mesh.indices.size() == 0;
			if (no_data) continue;

			mesh_count += model.instance_count;

			// Move vertex data
			uint32_t offset = static_cast<uint32_t>(scene_vertices.size());
			glm::vec3 sum = glm::vec3(0);
			float max_distance_from_center = -1;

			for (const Vertex& v : mesh.vertices) {
				scene_vertices.push_back(v);
				sum += v.position;

				float distance_from_center = glm::distance(v.position, glm::vec3(0, 0, 0));

				if (distance_from_center > max_distance_from_center) {
					max_distance_from_center = distance_from_center;
				}
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

			float length = static_cast<float>(mesh.vertices.size());
			glm::vec4 mesh_local_center_point = glm::vec4(sum.x / length, sum.y / length, sum.z / length, 1);

			// Find model center and instance data
			for (int i = 0; i < static_cast<int>(model.instance_count); i++){
				
				const glm::mat4& instance_model_matrix = model.instance_model_matrices[i];

				glm::vec4 mesh_world_center_point = instance_model_matrix[3] + mesh_local_center_point;
				mesh_world_center_point.w = 1;

				BoundingBoxData mesh_bounding_box;
				mesh_bounding_box.center_point = mesh_world_center_point;
				mesh_bounding_box.radius = glm::vec4(max_distance_from_center,0,0,0);
				bounding_data.push_back(mesh_bounding_box);

				instance_data.push_back({ instance_model_matrix , glm::vec4(0)});
			}
		}
	}

	std::vector<InstanceData> SceneParser::GetInstanceData() {
		return instance_data;
	}

	std::vector<BoundingBoxData> SceneParser::GetBoundingData(){
		return bounding_data;
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