#include "GraphicPipeline.h"

namespace FRS {

	void CreateGraphicPipeline(GraphicPipeline* pipe, 
		Device device,
		Swapchain swapChain,
		Shader shader,
		std::vector<Buffer> &buffers) {

		pipe->device = device;
		pipe->buffers = buffers;
		
		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = swapChain.format.format;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDependency subpassDepency = {};

		subpassDepency.srcSubpass = VK_SUBPASS_EXTERNAL;
		subpassDepency.dstSubpass = 0;
		subpassDepency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpassDepency.srcAccessMask = 0;
		subpassDepency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpassDepency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkSubpassDescription des = {};
		des.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		des.colorAttachmentCount = 1;
		des.pColorAttachments = &colorAttachmentRef;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &des;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &subpassDepency;

		VkResult res2 = vkCreateRenderPass(device.logicalDevice, &renderPassInfo, nullptr, &pipe->mRenderPass);

		FRS_S_ASSERT(res2 != VK_SUCCESS);

		VkPipelineShaderStageCreateInfo vShaderStage = {};

		vShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vShaderStage.module = shader.GetVertexModule();
		vShaderStage.pName = "main";
		vShaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;

		VkPipelineShaderStageCreateInfo fShaderStage = {};

		fShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fShaderStage.module = shader.GetFragmentModule();
		fShaderStage.pName = "main";
		fShaderStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkPipelineShaderStageCreateInfo shaderStages[] = {vShaderStage,fShaderStage};

		const uint32_t sizes = buffers.size();

		std::vector<VkVertexInputBindingDescription> descriptions;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

		for (auto& buffer : buffers) {
			descriptions.push_back(buffer.GetBindingDescriptions());
			
			for (auto& attributeInfo : buffer.GetAttributeDescriptions()) {
				attributeDescriptions.push_back(attributeInfo);
			}
		}

		std::cout << "DES: "<<descriptions.size() << std::endl;

		VkPipelineVertexInputStateCreateInfo vertexInput = {};
		vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInput.vertexAttributeDescriptionCount = attributeDescriptions.size();
		vertexInput.vertexBindingDescriptionCount = buffers.size();
		vertexInput.pVertexAttributeDescriptions = attributeDescriptions.data();
		vertexInput.pVertexBindingDescriptions = descriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAsm = {};
		inputAsm.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAsm.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAsm.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport = {};

		viewport.x = 0;
		viewport.y = 0;
		viewport.width = swapChain.swapChainExtent.width;
		viewport.height = swapChain.swapChainExtent.height;
		viewport.minDepth = 0;
		viewport.maxDepth = 1;

		VkRect2D scissors = {};

		scissors.offset = { 0,0 };
		scissors.extent = swapChain.swapChainExtent;

		VkPipelineViewportStateCreateInfo viewportInf = {};
		viewportInf.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportInf.viewportCount = 1;
		viewportInf.pViewports = &viewport;
		viewportInf.scissorCount = 1;
		viewportInf.pScissors = &scissors;

		VkPipelineRasterizationStateCreateInfo rasterizationInf = {};
		rasterizationInf.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationInf.depthClampEnable = VK_FALSE;
		rasterizationInf.rasterizerDiscardEnable = VK_FALSE;
		rasterizationInf.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationInf.lineWidth = 1.0f;
		rasterizationInf.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizationInf.frontFace = VK_FRONT_FACE_CLOCKWISE;

		rasterizationInf.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multiSampling = {};
		multiSampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multiSampling.sampleShadingEnable = VK_FALSE;
		multiSampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineColorBlendAttachmentState blendState = {};
		blendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
			VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;

		//rColor = newAlpha * newColor + (1-newAlpha)*oldColor
		//rAlpha = 1* newAlpha.a + 0* oldAlpha.a

		blendState.blendEnable = VK_FALSE;
		blendState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		blendState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blendState.colorBlendOp = VK_BLEND_OP_ADD;
		blendState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		blendState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		blendState.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo blendInfo = {};
		blendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		blendInfo.attachmentCount = 1;
		blendInfo.logicOpEnable = VK_FALSE;
		blendInfo.logicOp = VK_LOGIC_OP_COPY;
		blendInfo.pAttachments = &blendState;
		blendInfo.blendConstants[0] = 0;
		blendInfo.blendConstants[1] = 0;
		blendInfo.blendConstants[2] = 0;
		blendInfo.blendConstants[3] = 0;

		VkDynamicState states[] = {VK_DYNAMIC_STATE_LINE_WIDTH,
		VK_DYNAMIC_STATE_VIEWPORT};

		VkPipelineDynamicStateCreateInfo dynamicState = {};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = 2;
		dynamicState.pDynamicStates = states;

		VkPipelineLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		
		if (vkCreatePipelineLayout(device.logicalDevice, &layoutInfo, nullptr, &pipe->mLayout) != VK_SUCCESS) {
			throw std::runtime_error("Cant create layout!");
		}

		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	 	pipelineInfo.layout = pipe->mLayout;
		pipelineInfo.pInputAssemblyState = &inputAsm;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInput;
		pipelineInfo.pViewportState = &viewportInf;
		pipelineInfo.pRasterizationState = &rasterizationInf;
		pipelineInfo.pDynamicState = nullptr;
		pipelineInfo.pMultisampleState = &multiSampling;
		pipelineInfo.pColorBlendState = &blendInfo;
	    pipelineInfo.renderPass = pipe->mRenderPass;
	    pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;

		VkResult res = vkCreateGraphicsPipelines(device.logicalDevice,
			VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipe->pipeline);
		
		FRS_ASSERT_WV(res != VK_SUCCESS, "Cant create graphic pipe", res, 0);

		
	}

	void DestroyGraphicPipeline(
		GraphicPipeline pipe){

		vkDestroyPipelineLayout(pipe.device.logicalDevice,
			pipe.mLayout, nullptr);
		vkDestroyRenderPass(pipe.device.logicalDevice, 
			pipe.mRenderPass,
			nullptr);
		vkDestroyPipeline(pipe.device.logicalDevice,
			pipe.pipeline, nullptr);
	
	}

}