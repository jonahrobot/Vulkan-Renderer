#include "USDParser.h"
#include <fstream>
#include <iostream>
#include <random>
#include <chrono>
#include <thread>

namespace {

	uint32_t ReadUnsignedInt32(const std::vector<std::uint8_t>& ByteData, uint32_t Offset, uint32_t BufferSize)
	{
		const size_t byte_count = sizeof(uint32_t);

		if (Offset + byte_count > BufferSize) {
			throw std::runtime_error("Out of bounds");
		}

		uint32_t x;
		std::memcpy(&x, ByteData.data() + Offset, byte_count);

		return x;
	}

	template <class T>
	std::vector<T> ReadArray(const std::vector<std::uint8_t>& ByteData, uint32_t Offset, uint32_t BufferSize, uint32_t OutputArraySize) {

		const size_t byte_count = OutputArraySize * sizeof(T);

		if (Offset + byte_count > BufferSize) {
			throw std::runtime_error("Out of bounds");
		}

		std::vector<T> x(OutputArraySize);
		std::memcpy(x.data(), ByteData.data() + Offset, byte_count);

		return x;
	}

	inline std::vector<float> ReadFloatArray(const std::vector<std::uint8_t>& ByteData, uint32_t Offset, uint32_t BufferSize, uint32_t OutputArraySize) {
		return ReadArray<float>(ByteData, Offset, BufferSize, OutputArraySize);
	}

	inline std::vector<uint16_t> ReadUnsignedInt16Array(const std::vector<std::uint8_t>& ByteData, uint32_t Offset, uint32_t BufferSize, uint32_t OutputArraySize) {
		return ReadArray<uint16_t>(ByteData, Offset, BufferSize, OutputArraySize);
	}

	void ReadModelData(const std::vector<std::uint8_t>& Buffer, const std::vector<uint32_t>& ObjectPointers, std::vector<renderer::detail::InstanceModelData>& OutputData, uint32_t ChunkSize, uint32_t TID, uint32_t BufferSize, uint32_t ModelCount) {

		uint32_t start = ChunkSize * TID;
		uint32_t end = std::min(start + ChunkSize, ModelCount);

		std::mt19937 rng(12345);
		std::uniform_real_distribution<float> dist(0.2f, 1.0f);

		std::vector<float> vertices(0);
		std::vector<uint16_t> indices(0);
		std::vector<float> normals(0);
		std::vector<float> matrices(0);

		std::vector<renderer::detail::InstanceModelData> local_model_data;

		for (int i = start; i < end; i++) {

			// Get Object start byte
			uint32_t object_pointer = ObjectPointers[i];
			uint32_t byte_offset = object_pointer;

			// Parse object header
			uint32_t vertex_count = ReadUnsignedInt32(Buffer, byte_offset, BufferSize);
			byte_offset += sizeof(uint32_t);
			uint32_t index_count = ReadUnsignedInt32(Buffer, byte_offset, BufferSize);
			byte_offset += sizeof(uint32_t);
			uint32_t normal_count = ReadUnsignedInt32(Buffer, byte_offset, BufferSize);
			byte_offset += sizeof(uint32_t);
			uint32_t instance_count = ReadUnsignedInt32(Buffer, byte_offset, BufferSize);
			byte_offset += sizeof(uint32_t);

			// Parse object data
			vertices.resize(vertex_count * 3);
			indices.resize(index_count * 3);
			normals.resize(normal_count);
			matrices.resize(instance_count * 16);

			vertices = ReadFloatArray(Buffer, byte_offset, BufferSize, vertex_count * 3);
			byte_offset += vertex_count * 3 * sizeof(float);

			indices = ReadUnsignedInt16Array(Buffer, byte_offset, BufferSize, index_count);
			byte_offset += index_count * sizeof(uint16_t);

			normals = ReadFloatArray(Buffer, byte_offset, BufferSize, normal_count * 3);
			byte_offset += normal_count * 3 * sizeof(float);

			matrices = ReadFloatArray(Buffer, byte_offset, BufferSize, instance_count * 16);

			// Create model object
			renderer::detail::InstanceModelData new_model;
			new_model.instance_count = instance_count;

			glm::vec3 mesh_color = { dist(rng), dist(rng), dist(rng) };

			for (int j = 0; j < vertices.size(); j += 3) {
				renderer::detail::Vertex v;
				v.color = mesh_color;
				v.position = { vertices[j], vertices[j + 1],vertices[j + 2] };

				new_model.model_data.vertices.push_back(v);
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
			}

			local_model_data.push_back(new_model);
		}

		OutputData = std::move(local_model_data);
	}

	std::vector<std::uint8_t> ReadRest(std::ifstream& File) {

		// Get remaining size leftover
		std::streampos file_mark_current = File.tellg();
		File.seekg(0, std::ios::end);
		std::streampos file_mark_end = File.tellg();
		File.seekg(file_mark_current, std::ios::beg);
		std::size_t remaining_file_size = static_cast<std::size_t>(file_mark_end - file_mark_current);

		// Create byte array
		std::vector<std::uint8_t> remaining_bytes(remaining_file_size);

		// Read data
		File.read(reinterpret_cast<char*>(remaining_bytes.data()), remaining_file_size);

		return remaining_bytes;
	}

	std::vector<renderer::detail::InstanceModelData> Run_ParseUSD(std::string MP_FilePath) {
		std::ifstream file(MP_FilePath, std::ios::binary);

		if (!file) {
			throw std::invalid_argument("File could not open.");
		}

		uint64_t total_unique_objects = 0;

		// Get header data
		uint32_t model_count;
		file.read(reinterpret_cast<char*>(&model_count), sizeof(uint32_t));

		std::vector<uint32_t> model_pointers(model_count);
		file.read(reinterpret_cast<char*>(model_pointers.data()), model_count * sizeof(uint32_t));

		// Get rest of object data as byte array
		std::vector<std::uint8_t> remaining_bytes = ReadRest(file);
		uint32_t data_size = static_cast<uint32_t>(remaining_bytes.size());

		// Prepare for thread launch
		uint32_t max_thread_count = std::thread::hardware_concurrency();
		uint8_t thread_count = static_cast<uint8_t>(std::min(max_thread_count, model_count));
		uint32_t chunk_size = (model_count + (thread_count - 1)) / thread_count;

		// Launch threads
		std::vector<std::thread> threads;
		threads.reserve(thread_count);

		std::vector<std::vector<renderer::detail::InstanceModelData>> output_data(thread_count);

		std::cout << "Launched: " << static_cast<int>(thread_count) << " threads with a chunk size of " << chunk_size << "." << std::endl;

		for (int i = 0; i < thread_count; i++) {
			threads.emplace_back(ReadModelData, std::cref(remaining_bytes), std::cref(model_pointers), std::ref(output_data[i]), chunk_size, i, data_size, model_count);
		}

		// Wait for threads to join
		for (std::thread& t : threads) {
			if (t.joinable()) {
				t.join();
			}
		}

		// Threads complete merge workload
		std::vector<renderer::detail::InstanceModelData> merged_object_data;
		for (auto& vector : output_data) {
			merged_object_data.insert(merged_object_data.end(),
				std::make_move_iterator(vector.begin()),
				std::make_move_iterator(vector.end()));
		}

		return merged_object_data;
	}
} // namespace unnamed

namespace USD {

	std::vector<renderer::detail::InstanceModelData> ParseUSD(std::string MP_FilePath, bool BenchmarkMode){
		
		if (BenchmarkMode == false) {
			return Run_ParseUSD(MP_FilePath);
		}

		int run_count = 10;
		long long total = 0;

		for (int i = 0; i < run_count; i++) {
			auto start = std::chrono::high_resolution_clock::now();

			Run_ParseUSD(MP_FilePath);

			auto end = std::chrono::high_resolution_clock::now();
			auto execution_time_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
			total += execution_time_us;
		}

		auto average_time = total / static_cast<long long>(run_count);

		std::cout << "Average time over " << run_count << " executions: ";
		std::cout << average_time / 60000000 << "m " << (average_time / 1000000) % 60 << "s " << (average_time / 1000) % 1000 << "ms " << average_time % 1000 << "us" << std::endl;
	
		return Run_ParseUSD(MP_FilePath);
	}

} // namespace USD