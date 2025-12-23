#include "USDParser.h"
#include <JSON/json.hpp>
#include <fstream>
#include <iostream>
#include <random>

namespace USD {

	std::vector<renderer::detail::ModelWithUsage> ParseUSD(std::string json_file_path){

		std::ifstream json_file_stream(json_file_path);
		nlohmann::ordered_json scene_data = nlohmann::ordered_json::parse(json_file_stream);

		std::vector<renderer::detail::ModelWithUsage> output_data;
	
		auto model_data = scene_data["models"];

		std::mt19937 rng(12345);
		std::uniform_real_distribution<float> dist(0.2f, 1.0f);

		uint64_t model_load_count = 0;

		for (auto model_set : model_data.items()) {
			
			glm::vec3 mesh_color = { dist(rng), dist(rng), dist(rng)};

			renderer::detail::ModelWithUsage new_model;
			new_model.model_name = model_set.key();
			auto model = model_set.value();

			std::vector<float> vertices = model["vertices"].get<std::vector<float>>();
			
			for (int j = 0; j < vertices.size(); j+=3) {
				renderer::detail::Vertex v;
				v.color = mesh_color;
				v.position = {vertices[j], vertices[j+1],vertices[j+2] };
				v.tex_coord = { 0,0 };
;				new_model.model_data.vertices.push_back(v);
			}

			std::vector<uint32_t> indices = model["indices"].get<std::vector<uint32_t>>();

			for (int j = 0; j < indices.size(); j += 1) {
				new_model.model_data.indices.push_back(indices[j]);
			}

			new_model.instance_count = model["instance_count"].get<uint32_t>();

			auto instances = model["instances"];

			for (int j = 0; j < new_model.instance_count; j++) {
				// Note GLM matrices are Column-Major!

				auto instance = instances[j];
				auto row_one = instance[0].get<std::vector<float>>();
				auto row_two = instance[1].get<std::vector<float>>();
				auto row_three = instance[2].get<std::vector<float>>();
				auto row_four = instance[3].get<std::vector<float>>();

				glm::mat4 instance_matrix(
					row_one[0], row_one[1], row_one[2], row_one[3], // Column 0
					row_two[0], row_two[1], row_two[2], row_two[3], // Column 1
					row_three[0], row_three[1], row_three[2], row_three[3], // Column 2
					row_four[0], row_four[1], row_four[2], 1// Column 3
				);

				new_model.instance_model_matrices.push_back(instance_matrix);
				model_load_count += 1;
			}	

			output_data.push_back(new_model);
		}

		std::cout << "On load total models x instances is = " << model_load_count << std::endl;

		return output_data;
	}

} // namespace USD