#pragma once
#include <string>
#include "../Renderer/Detail/RendererDetail.h"

namespace USD {

	std::vector<renderer::detail::ModelData> ParseUSD(std::string usd_root_file_path);

} // namespace USD