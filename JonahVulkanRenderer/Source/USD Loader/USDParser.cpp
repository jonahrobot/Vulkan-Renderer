#include "USDParser.h"
#include <fstream>
#include <iostream>
#include <random>
#include <chrono>

namespace USD {

	struct obj_header {
		uint32_t num_vertices, num_indices, num_normals, num_instances;
	};

	std::vector<renderer::detail::InstanceModelData> ParseUSD(std::string mp_file_path){

		std::ifstream file(mp_file_path, std::ios::binary);

		if (!file) {
			throw std::invalid_argument("File could not open.");
		}

		auto start = std::chrono::high_resolution_clock::now();

		std::vector<renderer::detail::InstanceModelData> output_data;
		std::mt19937 rng(12345);
		std::uniform_real_distribution<float> dist(0.2f, 1.0f);

		uint64_t total_unique_objects = 0;

		// Get header data
		uint32_t model_count;
		file.read(reinterpret_cast<char*>(&model_count), sizeof(uint32_t));

		std::vector<uint32_t> model_pointers(model_count);
		file.read(reinterpret_cast<char*>(model_pointers.data()), model_count * sizeof(uint32_t));

		std::vector<float> vertices(0);
		std::vector<uint16_t> indices(0);
		std::vector<float> normals(0);
		std::vector<float> matrices(0);

		for (int i = 0; i < model_count; i++) {

			renderer::detail::InstanceModelData new_model;

			obj_header header;
			file.read(reinterpret_cast<char*>(&header), sizeof(header));

			vertices.resize(header.num_vertices * 3);
			indices.resize(header.num_indices);
			normals.resize(header.num_normals * 3);
			matrices.resize(header.num_instances * 16);

			new_model.instance_count = header.num_instances;

			file.read(reinterpret_cast<char*>(vertices.data()), header.num_vertices * 3 * sizeof(float));
			file.read(reinterpret_cast<char*>(indices.data()), header.num_indices * sizeof(uint16_t));
			file.read(reinterpret_cast<char*>(normals.data()), header.num_normals * 3 * sizeof(float));
			file.read(reinterpret_cast<char*>(matrices.data()), header.num_instances * 16 * sizeof(float));

			glm::vec3 mesh_color = { dist(rng), dist(rng), dist(rng) };
			
			for (int j = 0; j < vertices.size(); j+=3) {
				renderer::detail::Vertex v;
				v.color = mesh_color;
				v.position = {vertices[j], vertices[j+1],vertices[j+2] };
				v.tex_coord = { 0,0 };
;				new_model.model_data.vertices.push_back(v);
			}

			for (int j = 0; j < indices.size(); j += 1) {
				new_model.model_data.indices.push_back(indices[j]);
			}

			for (int j = 0; j < matrices.size(); j += 16) {
				// Note GLM matrices are Column-Major!

				glm::mat4 instance_matrix(
					matrices[j + 0], matrices[j + 1], matrices[j + 2], matrices[j + 3],		// Column 0
					matrices[j + 4], matrices[j + 5], matrices[j + 6], matrices[j + 7],		// Column 1
					matrices[j + 8], matrices[j + 9], matrices[j + 10], matrices[j + 11],	// Column 2
					matrices[j + 12], matrices[j + 13], matrices[j + 14], 1.0f				// Column 3
				);

				new_model.instance_model_matrices.push_back(instance_matrix);
				total_unique_objects += 1;
			}	

			output_data.push_back(new_model);
		}

		std::cout << "On load total models x instances is = " << total_unique_objects << std::endl;

		auto end = std::chrono::high_resolution_clock::now();
		auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
		std::cout << us / 60000000 << "m " << (us / 1000000) % 60 << "s "
			<< (us / 1000) % 1000 << "ms " << us % 1000 << "us" << std::endl;

		return output_data;
	}

} // namespace USD