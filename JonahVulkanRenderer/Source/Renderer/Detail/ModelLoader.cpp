#include <iostream>
#include "RendererDetail.h"

// Unnamed namespace to show functions below are pure Utility strictly for this .cpp file.
namespace {
}

// Implements all Image Loading functions in "RendererDetail.h" to be used in "Renderer.cpp"
namespace renderer::detail {

	ModelData LoadModel(std::string ModelPath) {

		ModelData new_model;

		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string err;
		std::string warn;

		if (tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, ModelPath.c_str()) == false) {
			throw std::runtime_error(err);
		}

		std::unordered_map<detail::Vertex, uint32_t> unique_vertices{};

		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				detail::Vertex vertex{};

				vertex.position = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				vertex.tex_coord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				};

				vertex.color = { 1.0f, 1.0f, 1.0f };

				// If vertex not been acounted for then add it!
				if (unique_vertices.count(vertex) == 0) {
					unique_vertices[vertex] = static_cast<uint32_t> (new_model.vertices_to_render.size());
					new_model.vertices_to_render.push_back(vertex);
				}

				// Find vertex's index and add it to indices array
				new_model.indices.push_back(unique_vertices[vertex]);
			}
		}

		return new_model;
	}

} // namespace renderer::detail