#include "GraphicPipeline.h"

namespace FRS {

	std::vector<Buffer> CreateVertexBufferFromShader(Device device,
		DeviceAllocator allocator,
		Shader shader,
		uint32_t totalSize) {

		std::vector<Buffer> tBuffers;

		for (uint32_t i = 0; i < totalSize; i++) {
			Buffer buffer{};

			CreateBuffer(buffer, device, VK_BUFFER_USAGE_TRANSFER_DST_BIT|
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				shader.VertexInput.BindingSize[i], true,
				allocator);

			buffer.directData = shader.VertexInput.BindingDatas[i];
			buffer.transferSize = shader.VertexInput.BindingSize[i];

			tBuffers.push_back(buffer);
		}

		return tBuffers;
	}


	void CreateUniformBufferFromShader(Device device,
		DeviceAllocator allocator,
		Shader shader,
		GraphicPipeline* pipe,
		uint32_t& totalSet,
		std::vector<uint32_t>& sizePerSet,
		std::vector<uint32_t>& sizeTexPerSet) {

		std::vector<Buffer> tBuffers;
		std::vector<Texture> tTextures;
		std::vector<VkDescriptorSetLayoutBinding> setLayouts;
		std::vector<VkDescriptorPoolSize> setPools;

		uint32_t i;

		for (i = 0; i < 25; i++) {
				
			if (shader.UniformSets[i].UniformBindings[0].stage == UNDENTIFIED)
				break;

			else {

				uint32_t j = 0;

				std::vector<Buffer> GroupBuffer;
				std::vector<Texture> GroupTextures;

				uint32_t bufferSize = 0;
				uint32_t texSize = 0;

				for (j = 0; j < 25; j++) {

					if (shader.UniformSets[i].UniformBindings[j].stage == UNDENTIFIED)
						break;
					else {
						if (shader.UniformSets[i].UniformBindings[j].Type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
							
							bufferSize++;

							Buffer bufferz{};

							CreateBuffer(bufferz, device, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
								shader.UniformSets[i].BindingSize[j], true,
								allocator);

							bufferz.length = shader.UniformSets[i].UniformBindings[j].dataArrayLength;

							for (uint32_t k = 0; k < shader.UniformSets[i].UniformBindings[j].dataArrayLength; k++) {
								
								bufferz.range[k] = shader.UniformSets[i].UniformBindings[j].Range[k];
								bufferz.offset[k] = shader.UniformSets[i].UniformBindings[j].OffSet[k];

							}

							VkDescriptorSetLayoutBinding layout{};

							layout.binding = j;
							layout.descriptorCount = shader.UniformSets[i].UniformBindings[j].dataArrayLength;
							layout.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
							layout.stageFlags = shader.UniformSets[i].UniformBindings[j].stage;
							layout.pImmutableSamplers = nullptr;

							bufferz.setLayoutBinding = layout;

							VkDescriptorPoolSize poolSize{};

							poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
							poolSize.descriptorCount = shader.UniformSets[i].UniformBindings[j].dataArrayLength;
							
							bufferz.setPoolSize = poolSize;

							tBuffers.push_back(bufferz);
							GroupBuffer.push_back(bufferz);
						}
						else
							if (shader.UniformSets[i].UniformBindings[j].Type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
								
								texSize++;
								Texture* tex = static_cast<Texture*>(shader.UniformSets[i].BindingDatas[j]);

								VkDescriptorSetLayoutBinding layout = {};

								if (shader.UniformSets[i].UniformBindings[j].dataArrayLength != tex->GetSamplers().size()) {
									throw std::runtime_error("Please watch out for the data length. Image already has mipmap");
								}

								layout.binding = j;
								layout.descriptorCount = shader.UniformSets[i].UniformBindings[j].dataArrayLength;
								layout.descriptorType = shader.UniformSets[i].UniformBindings[j].Type;
								layout.stageFlags = shader.UniformSets[i].UniformBindings[j].stage;
								layout.pImmutableSamplers = nullptr;

								tex->setUniformLayoutBinding = layout;

								tex->poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
								tex->poolSize.descriptorCount = shader.UniformSets[i].UniformBindings[j].dataArrayLength;

								tTextures.push_back(*tex);
								GroupTextures.push_back(*tex);
							}
						
					}
	
				}

				sizeTexPerSet.push_back(texSize);
				sizePerSet.push_back(bufferSize);
				pipe->CreateUniformDescriptorSetLayout(tBuffers, tTextures, i);
			}
			
		}

		totalSet = i;

		pipe->uniformBuffers = tBuffers;
		pipe->textures = tTextures;

	}

	std::vector<Buffer> CreateIndexbuffer(Device device, Shader shader,
		DeviceAllocator allocator) {
		std::vector<Buffer> tBuffers;

		for (uint32_t i = 0; i < 25; i++) {
			if (shader.IndexInput.IndexDatas[i] == nullptr) {
				break;
			}
			else {

				Buffer buff;
				CreateBuffer(buff, device, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
					VK_BUFFER_USAGE_INDEX_BUFFER_BIT, shader.IndexInput.IndexSize[i],
					true, allocator);

				buff.directData = shader.IndexInput.IndexDatas[i];
				buff.transferSize = shader.IndexInput.IndexSize[i];

				tBuffers.push_back(buff);
			}
		}

		return tBuffers;
	}

	void CreateGraphicPipeline(GraphicPipeline* pipe,
		Device device,
		DeviceAllocator allocator,
		Swapchain swapChain,
		Shader shader) {

		bool haveDestroy = false;

		if (pipe->mLayout != VK_NULL_HANDLE) {
			vkDestroyPipelineLayout(pipe->device.logicalDevice,
				pipe->mLayout, nullptr);

			haveDestroy = true;
		}
		
		if (pipe->mRenderPass != VK_NULL_HANDLE) {
			vkDestroyRenderPass(pipe->device.logicalDevice,
				pipe->mRenderPass,
				nullptr);
		}

		if (pipe->pipeline != VK_NULL_HANDLE) {
			vkDestroyPipeline(pipe->device.logicalDevice,
				pipe->pipeline, nullptr);
		}

		for (uint32_t i = 0; i < pipe->uniformDesLayouts.size() && pipe->uniformDesLayouts.size() != 0; i++) {
			vkDestroyDescriptorSetLayout(pipe->device.logicalDevice,
				pipe->uniformDesLayouts[i], nullptr);

			pipe->uniformDesLayouts[i] = VK_NULL_HANDLE;
		}

		pipe->uniformDesLayouts.resize(0);
		pipe->uniformPoolSizes.resize(0);

		pipe->desBufferHandle.clear();
		pipe->uniformWriteDescriptorSets.clear();

		pipe->device = device;
		pipe->mLayout = VK_NULL_HANDLE;
		pipe->allocator = allocator;

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

		std::vector<VkVertexInputAttributeDescription> attributesDes;
		std::vector<VkVertexInputBindingDescription> bindingDes;

		FRS::GetShaderBind(shader, attributesDes, bindingDes);

		if (haveDestroy == false) {

			pipe->staticBuffers = FRS::CreateVertexBufferFromShader(device, allocator,
				shader, bindingDes.size());
			FRS::CreateUniformBufferFromShader(device, allocator, shader,
				pipe, pipe->totalSet, pipe->sizePerSet, pipe->sizeTexPerSet);
			pipe->indexBuffers = FRS::CreateIndexbuffer(device, shader, allocator);

			for (auto& buffer : pipe->staticBuffers) {
				pipe->realBuffers.push_back(buffer);
			}

			for (auto& buffer : pipe->uniformBuffers) {
				pipe->realBuffers.push_back(buffer);
			}

			for (auto& buffer : pipe->indexBuffers) {
				pipe->realBuffers.push_back(buffer);
			}

		}
		else {
			for (uint32_t i = 0; i < pipe->totalSet; i++) {
				std::vector<Buffer> buffers;
				std::vector<Texture> textures;

				for (uint32_t j = 0; j < pipe->sizePerSet[i]; j++) {
					buffers.push_back(pipe->uniformBuffers[(i + 1) * j]);
				}

				for (uint32_t j = 0; j < pipe->sizeTexPerSet[i]; j++) {
					textures.push_back(pipe->textures[(i + 1) * j]);
				}

				pipe->CreateUniformDescriptorSetLayout(buffers, textures,
					i);
			}
		}
    
		VkPipelineVertexInputStateCreateInfo vertexInput = {};
		vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInput.vertexAttributeDescriptionCount = attributesDes.size();
		vertexInput.vertexBindingDescriptionCount = bindingDes.size();
		vertexInput.pVertexAttributeDescriptions = attributesDes.data();
		vertexInput.pVertexBindingDescriptions = bindingDes.data();

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
		rasterizationInf.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

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
		layoutInfo.setLayoutCount = pipe->uniformDesLayouts.size();
		layoutInfo.pSetLayouts = pipe->uniformDesLayouts.data();

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
		GraphicPipeline* pipe){

		vkDestroyPipelineLayout(pipe->device.logicalDevice,
			pipe->mLayout, nullptr);
		vkDestroyRenderPass(pipe->device.logicalDevice,
			pipe->mRenderPass,
			nullptr);
		vkDestroyPipeline(pipe->device.logicalDevice,
			pipe->pipeline, nullptr);

		for (uint32_t i = 0; i < pipe->uniformDesLayouts.size(); i++) {
			vkDestroyDescriptorSetLayout(pipe->device.logicalDevice,
				pipe->uniformDesLayouts[i], nullptr);
			pipe->uniformDesLayouts[i] = VK_NULL_HANDLE;
		}

		pipe->uniformDesLayouts.resize(0);
		pipe->uniformPoolSizes.resize(0);

		pipe->desBufferHandle.clear();
		pipe->uniformWriteDescriptorSets.clear();

		for (auto& buffer : pipe->realBuffers) {
			DestroyBuffer(pipe->device, &buffer);
		}
	}

}