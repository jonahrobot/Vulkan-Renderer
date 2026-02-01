#pragma once
#include <string>
#include "../Renderer/Detail/RendererDetail.h"

namespace USD {

	std::vector<renderer::detail::MeshInstances> ParseUSD(std::string json_file_path, bool BenchmarkMode = false);

} // namespace USD