#pragma once
#include <string>
#include "../Renderer/VkUtil/VkCommon.h"

namespace MP {

	std::vector<renderer::MeshInstances> ParseMP(std::string json_file_path, bool BenchmarkMode = false);

} // namespace MP