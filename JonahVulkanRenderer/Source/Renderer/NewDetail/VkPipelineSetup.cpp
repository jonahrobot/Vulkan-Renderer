#include <string>
#include <fstream>
#include <iostream>

#include "VkPipelineSetup.h"
#include "VkCommon.h"

namespace {

	// Used by CreateGraphicsPipeline() and CreateComputePipeline()
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

	// Used by CreateGraphicsPipeline() and CreateComputePipeline()
	VkShaderModule CreateShaderModule(const std::vector<char>& ShaderBinary, const VkDevice LogicalDevice) {

		VkShaderModuleCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		create_info.codeSize = ShaderBinary.size();
		create_info.pCode = reinterpret_cast<const uint32_t*>(ShaderBinary.data());

		VkShaderModule shader_module;
		VkResult created = vkCreateShaderModule(LogicalDevice, &create_info, nullptr, &shader_module);

		if (created != VK_SUCCESS) {
			throw std::runtime_error("Failed to create shader module.");
		}

		return shader_module;
	}
}

namespace renderer::pipeline {

	VkRenderPass CreateRenderPass(VkDevice LogicalDevice, VkPhysicalDevice PhysicalDevice, VkFormat SwapchainFormat, VkFormat DepthBufferFormat) {
		VkRenderPass render_pass;

		VkAttachmentDescription color_attachment{};
		color_attachment.format = SwapchainFormat;
		color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference color_attachment_reference{};
		color_attachment_reference.attachment = 0;
		color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depth_attachment{};
		depth_attachment.format = DepthBufferFormat;
		depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depth_attachment_reference{};
		depth_attachment_reference.attachment = 1;
		depth_attachment_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_attachment_reference;
		subpass.pDepthStencilAttachment = &depth_attachment_reference;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		std::array<VkAttachmentDescription, 2> attachments = { color_attachment, depth_attachment };

		VkRenderPassCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
		create_info.pAttachments = attachments.data();
		create_info.subpassCount = 1;
		create_info.pSubpasses = &subpass;
		create_info.dependencyCount = 1;
		create_info.pDependencies = &dependency;

		if (vkCreateRenderPass(LogicalDevice, &create_info, nullptr, &render_pass) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create render pass.");
		}

		return render_pass;
	}

#pragma region Descriptor Sets

	/*
	
		Future refactor: Pass in what type of Decriptor bindings we want from Renderer.cpp to remove hard coding!
	
	*/

	VkDescriptorSetLayout CreateDescriptorLayout(VkDevice LogicalDevice) {

		VkDescriptorSetLayoutBinding ubo{};
		ubo.binding = 0;
		ubo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		ubo.descriptorCount = 1;
		ubo.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT;
		ubo.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding instance_data{};
		instance_data.binding = 1;
		instance_data.descriptorCount = 1;
		instance_data.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		instance_data.pImmutableSamplers = nullptr;
		instance_data.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutBinding indirect_draw_commands{};
		indirect_draw_commands.binding = 2;
		indirect_draw_commands.descriptorCount = 1;
		indirect_draw_commands.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		indirect_draw_commands.pImmutableSamplers = nullptr;
		indirect_draw_commands.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

		VkDescriptorSetLayoutBinding mesh_centers{};
		mesh_centers.binding = 3;
		mesh_centers.descriptorCount = 1;
		mesh_centers.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		mesh_centers.pImmutableSamplers = nullptr;
		mesh_centers.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

		VkDescriptorSetLayoutBinding should_draw_flags{};
		should_draw_flags.binding = 4;
		should_draw_flags.descriptorCount = 1;
		should_draw_flags.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		should_draw_flags.pImmutableSamplers = nullptr;
		should_draw_flags.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT;

		std::array<VkDescriptorSetLayoutBinding, 5> bindings = { ubo, instance_data, indirect_draw_commands, mesh_centers, should_draw_flags };

		VkDescriptorSetLayoutCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		create_info.bindingCount = static_cast<uint32_t>(bindings.size());
		create_info.pBindings = bindings.data();

		VkDescriptorSetLayout descriptor_layout;
		if (vkCreateDescriptorSetLayout(LogicalDevice, &create_info, nullptr, &descriptor_layout) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create descriptor set layout.");
		}

		return descriptor_layout;
	}

	VkDescriptorPool CreateDescriptorPool(VkDevice LogicalDevice) {

		VkDescriptorPoolSize ubo;
		ubo.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		ubo.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		VkDescriptorPoolSize ssbo;
		ssbo.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		ssbo.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * 4;

		std::array<VkDescriptorPoolSize, 2> pools = { ubo, ssbo };

		VkDescriptorPoolCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		create_info.poolSizeCount = static_cast<uint32_t>(pools.size());
		create_info.pPoolSizes = pools.data();
		create_info.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

		VkDescriptorPool descriptor_pool;
		if (vkCreateDescriptorPool(LogicalDevice, &create_info, nullptr, &descriptor_pool) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create descriptor pool.");
		}

		return descriptor_pool;
	}

	std::vector<VkDescriptorSet> CreateDescriptorSets(VkDevice LogicalDevice, VkDescriptorSetLayout Layout, VkDescriptorPool Pool) {

		std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, Layout);

		VkDescriptorSetAllocateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		create_info.descriptorPool = Pool;
		create_info.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
		create_info.pSetLayouts = layouts.data();

		std::vector<VkDescriptorSet> descriptor_sets(MAX_FRAMES_IN_FLIGHT);
		if (vkAllocateDescriptorSets(LogicalDevice, &create_info, descriptor_sets.data()) != VK_SUCCESS) {
			throw std::runtime_error("Failed to allocate descriptor set.");
		}

		return descriptor_sets;
	}
#pragma endregion

#pragma Pipeline Setup

	VkPipelineLayout CreatePipelineLayout(VkDevice LogicalDevice, VkDescriptorSetLayout DescriptorLayout) {
		VkPipelineLayoutCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		create_info.setLayoutCount = 1;
		create_info.pSetLayouts = &DescriptorLayout;

		VkPipelineLayout pipeline_layout;
		if (vkCreatePipelineLayout(LogicalDevice, &create_info, nullptr, &pipeline_layout) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create a pipeline layout.");
		}
	}

	VkPipeline CreateGraphicsPipeline(VkDevice LogicalDevice, VkPipelineLayout Layout, VkRenderPass RenderPass, const char* VertexShaderPath, const char* FragmentShaderPath) {

		auto vertex_shader_binary = ReadFile(VertexShaderPath);
		auto fragment_shader_binary = ReadFile(FragmentShaderPath);

		VkShaderModule vertex_shader_module = CreateShaderModule(vertex_shader_binary, LogicalDevice);
		VkShaderModule fragment_shader_module = CreateShaderModule(fragment_shader_binary, LogicalDevice);

		VkPipelineShaderStageCreateInfo vertex_stage{};
		vertex_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertex_stage.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertex_stage.module = vertex_shader_module;
		vertex_stage.pName = "main";

		VkPipelineShaderStageCreateInfo fragment_stage{};
		fragment_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragment_stage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragment_stage.module = fragment_shader_module;
		fragment_stage.pName = "main";

		VkPipelineShaderStageCreateInfo shader_stages[] = { vertex_stage, fragment_stage };

		VkPipelineVertexInputStateCreateInfo vertex_input_info{};
		vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		auto binding_description = Vertex::GetBindingDescription();
		vertex_input_info.vertexBindingDescriptionCount = 1;
		vertex_input_info.pVertexBindingDescriptions = &binding_description;

		auto attribute_description = Vertex::GetAttributeDescription();
		vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_description.size());
		vertex_input_info.pVertexAttributeDescriptions = attribute_description.data();

		VkPipelineInputAssemblyStateCreateInfo input_assembly{};
		input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		input_assembly.primitiveRestartEnable = VK_FALSE;

		std::vector<VkDynamicState> dynamic_states = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamic_state{};
		dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_state.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
		dynamic_state.pDynamicStates = dynamic_states.data();

		VkPipelineViewportStateCreateInfo viewport_state{};
		viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_state.viewportCount = 1;
		viewport_state.scissorCount = 1;

		VkPipelineDepthStencilStateCreateInfo depth_stencil{};
		depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depth_stencil.depthTestEnable = VK_TRUE;
		depth_stencil.depthWriteEnable = VK_TRUE;
		depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depth_stencil.depthBoundsTestEnable = VK_FALSE;
		depth_stencil.stencilTestEnable = VK_FALSE;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f;
		rasterizer.depthBiasClamp = 0.0f;
		rasterizer.depthBiasSlopeFactor = 0.0f;

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f;
		multisampling.pSampleMask = nullptr;
		multisampling.alphaToCoverageEnable = VK_FALSE;
		multisampling.alphaToOneEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState color_blend_attachment{};
		color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		color_blend_attachment.blendEnable = VK_TRUE;
		color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo color_blending{};
		color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blending.logicOpEnable = VK_FALSE;
		color_blending.attachmentCount = 1;
		color_blending.pAttachments = &color_blend_attachment;

		VkGraphicsPipelineCreateInfo pipeline_info{};
		pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_info.stageCount = 2;

		pipeline_info.pStages = shader_stages;
		pipeline_info.pVertexInputState = &vertex_input_info;
		pipeline_info.pInputAssemblyState = &input_assembly;
		pipeline_info.pViewportState = &viewport_state;
		pipeline_info.pRasterizationState = &rasterizer;
		pipeline_info.pMultisampleState = &multisampling;
		pipeline_info.pDepthStencilState = &depth_stencil;
		pipeline_info.pColorBlendState = &color_blending;
		pipeline_info.pDynamicState = &dynamic_state;

		pipeline_info.layout = Layout;
		pipeline_info.renderPass = RenderPass;
		pipeline_info.subpass = 0;

		pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
		pipeline_info.basePipelineIndex = -1;

		VkPipeline graphics_pipeline;
		if (vkCreateGraphicsPipelines(LogicalDevice, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &graphics_pipeline) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create graphics pipeline.");
		}

		vkDestroyShaderModule(LogicalDevice, fragment_shader_module, nullptr);
		vkDestroyShaderModule(LogicalDevice, vertex_shader_module, nullptr);

		return graphics_pipeline;
	}

	VkPipeline CreateComputePipeline(VkDevice LogicalDevice, VkPipelineLayout Layout, const char* ComputeShaderPath) {

		auto compute_shader_binary = ReadFile(ComputeShaderPath);
		VkShaderModule compute_shader_module = CreateShaderModule(compute_shader_binary, LogicalDevice);

		VkPipelineShaderStageCreateInfo compute_shader_stage{};
		compute_shader_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		compute_shader_stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
		compute_shader_stage.module = compute_shader_module;
		compute_shader_stage.pName = "main";

		VkComputePipelineCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		create_info.layout = Layout;
		create_info.stage = compute_shader_stage;

		VkPipeline compute_pipeline;
		if (vkCreateComputePipelines(LogicalDevice, VK_NULL_HANDLE, 1, &create_info, nullptr, &compute_pipeline) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create compute pipeline.");
		}

		vkDestroyShaderModule(LogicalDevice, compute_shader_module, nullptr);

		return compute_pipeline;
	}

#pragma endregion
}