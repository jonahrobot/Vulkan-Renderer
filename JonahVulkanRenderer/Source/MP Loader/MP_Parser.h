#pragma once
#include <string>
#include "../Renderer/Detail/RendererDetail.h"

namespace MP {

	std::vector<renderer::detail::MeshInstances> ParseMP(std::string json_file_path, bool BenchmarkMode = false);

} // namespace MP