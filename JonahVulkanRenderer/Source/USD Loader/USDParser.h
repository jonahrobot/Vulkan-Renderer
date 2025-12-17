#pragma once
#include <string>
#include "../Renderer/Detail/RendererDetail.h"

namespace USD {

	std::vector<renderer::detail::ModelWithUsage> ParseUSD(std::string json_file_path);

} // namespace USD