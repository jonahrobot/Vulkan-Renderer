#include "RendererDetail.h"
#include <string>
#include <fstream>
#include <iostream>

// Unnamed namespace to show functions below are pure Utility strictly for this .cpp file.
namespace {

	std::vector<char> ReadFile(const std::string& FileName) {
		std::ifstream file(FileName, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("Failed to open file.");
		}

		size_t file_size = (size_t)file.tellg();
		std::vector<char> buffer(file_size);

		file.seekg(0);
		file.read(buffer.data(), file_size);

		file.close();

		return buffer;
	}

}

// Implements all Vulkan Graphics Pipeline Creation functions in "RendererDetail.h" to be used in "Renderer.cpp"
namespace renderer::detail {

	void CreateGraphicsPipeline() {

		auto vertShaderCode = ReadFile("shaders/vert.spv");
		auto fragShaderCode = ReadFile("shaders/frag.spv");

		std::cout << "Vert shader size: " << vertShaderCode.size() << std::endl;
		std::cout << "Frag shader size: " << fragShaderCode.size() << std::endl;
	}

}