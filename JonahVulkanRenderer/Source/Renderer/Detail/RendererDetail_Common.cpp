#include "RendererDetail_Common.h"
#include <iostream>

namespace renderer::detail {

	MergedIndexVertexBuffer MergeIndexVertexBuffer(const std::vector<Vertex>& V1, const std::vector<uint32_t>& I1, const std::vector<Vertex>& V2, const std::vector<uint32_t>& I2) {

		MergedIndexVertexBuffer MergedData{};

		MergedData.merged_vertex_buffer = V1;
		for (Vertex point : V2) {
			point.position.z += 1;
			MergedData.merged_vertex_buffer.push_back(point);
		}

		MergedData.merged_index_buffer = I1;
		uint32_t size_of_v1 = static_cast<uint32_t>(V1.size());
		for (uint32_t x : I2) {
			MergedData.merged_index_buffer.push_back(x + size_of_v1);
		}

		return MergedData;
	}
}