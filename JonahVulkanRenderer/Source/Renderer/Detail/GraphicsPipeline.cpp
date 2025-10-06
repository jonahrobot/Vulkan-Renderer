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

	VkShaderModule CreateShaderModule(const std::vector<char>& Code, const VkDevice PhysicalDevice) {

		VkShaderModuleCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		create_info.codeSize = Code.size();
		create_info.pCode = reinterpret_cast<const uint32_t*>(Code.data());

		VkShaderModule shader_module;
		VkResult created = vkCreateShaderModule(PhysicalDevice, &create_info, nullptr, &shader_module);

		if (created != VK_SUCCESS) {
			throw std::runtime_error("Failed to create shader module.");
		}

		return shader_module;
	}

}

// Implements all Vulkan Graphics Pipeline Creation functions in "RendererDetail.h" to be used in "Renderer.cpp"
namespace renderer::detail {

	void CreateGraphicsPipeline(const VkDevice PhysicalDevice) {

		// Vertex and Fragment shaders
		auto vert_shader_code = ReadFile("shaders/vert.spv");
		auto frag_shader_code = ReadFile("shaders/frag.spv");

		VkShaderModule vert_shader_module = CreateShaderModule(vert_shader_code, PhysicalDevice);
		VkShaderModule frag_shader_module = CreateShaderModule(frag_shader_code, PhysicalDevice);

		VkPipelineShaderStageCreateInfo vert_shader_stage_info{};
		vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vert_shader_stage_info.module = vert_shader_module;
		vert_shader_stage_info.pName = "main";

		VkPipelineShaderStageCreateInfo frag_shader_stage_info{};
		frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		frag_shader_stage_info.module = frag_shader_module;
		frag_shader_stage_info.pName = "main";

		VkPipelineShaderStageCreateInfo shader_stages[] = {vert_shader_stage_info, frag_shader_stage_info};

		// Dynamic state creation
		std::vector<VkDynamicState> dynamic_states = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamic_state{};
		dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_state.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
		dynamic_state.pDynamicStates = dynamic_states.data();

		// Cleanup
		vkDestroyShaderModule(PhysicalDevice, frag_shader_module, nullptr);
		vkDestroyShaderModule(PhysicalDevice, vert_shader_module, nullptr);
	}

}