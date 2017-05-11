#include "GraphicPipeline.h"

namespace FRS {

	std::vector<Buffer> CreateVertexBufferFromShader(Device device,
		DeviceAllocator* allocator,
		Shader* shader,
		uint32_t totalSize) {

		std::vector<Buffer> tBuffers;

		for (uint32_t i = 0; i < totalSize; i++) {
			Buffer buffer{};

			CreateBuffer(buffer, device, VK_BUFFER_USAGE_TRANSFER_DST_BIT|
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				shader->VertexInput.BindingSize[i], true,
				allocator);

			buffer.directData = shader->VertexInput.BindingDatas[i];
			buffer.transferSize = shader->VertexInput.BindingSize[i];

			tBuffers.push_back(buffer);
		}

		return tBuffers;
	}


	void CreateUniformBufferFromShader(Device device,
		DeviceAllocator* allocator,
		Shader* shader,
		GraphicPipeline* pipe,
		uint32_t& totalSet,
		std::vector<uint32_t>& sizePerSet,
		std::vector<uint32_t>& sizeTexPerSet) {

		std::vector<Buffer> tBuffers;
		std::vector<Texture*> tTextures;
		std::vector<VkDescriptorSetLayoutBinding> setLayouts;
		std::vector<VkDescriptorPoolSize> setPools;

		uint32_t i;

		for (i = 0; i < 25; i++) {
				
			if (shader->UniformSets[i].UniformBindings[0].stage == UNDENTIFIED)
				break;

			else {

				uint32_t j = 0;

				std::vector<Buffer> GroupBuffer;
				std::vector<Texture*> GroupTextures;

				uint32_t bufferSize = 0;
				uint32_t texSize = 0;

				for (j = 0; j < 25; j++) {

					if (shader->UniformSets[i].UniformBindings[j].stage == UNDENTIFIED)
						break;
					else {
						if (shader->UniformSets[i].UniformBindings[j].Type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
							
							bufferSize++;

							Buffer bufferz{};

							CreateBuffer(bufferz, device, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
								shader->UniformSets[i].BindingSize[j], true,
								allocator);

							bufferz.length = shader->UniformSets[i].UniformBindings[j].dataArrayLength;

							for (uint32_t k = 0; k < shader->UniformSets[i].UniformBindings[j].dataArrayLength; k++) {
								
								bufferz.range[k] = shader->UniformSets[i].UniformBindings[j].Range[k];
								bufferz.offset[k] = shader->UniformSets[i].UniformBindings[j].OffSet[k];

							}

							VkDescriptorSetLayoutBinding layout{};

							layout.binding = j;
							layout.descriptorCount = shader->UniformSets[i].UniformBindings[j].dataArrayLength;
							layout.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
							layout.stageFlags = shader->UniformSets[i].UniformBindings[j].stage;
							layout.pImmutableSamplers = nullptr;

							bufferz.setLayoutBinding = layout;

							VkDescriptorPoolSize poolSize{};

							poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
							poolSize.descriptorCount = shader->UniformSets[i].UniformBindings[j].dataArrayLength;
							
							bufferz.setPoolSize = poolSize;

							tBuffers.push_back(bufferz);
							GroupBuffer.push_back(bufferz);
						}
						else
							if (shader->UniformSets[i].UniformBindings[j].Type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
								
								texSize++;
								Texture* tex = static_cast<Texture*>(shader->UniformSets[i].BindingDatas[j]);

								VkDescriptorSetLayoutBinding layout = {};


								layout.binding = j;
								layout.descriptorCount = shader->UniformSets[i].UniformBindings[j].dataArrayLength;
								layout.descriptorType = shader->UniformSets[i].UniformBindings[j].Type;
								layout.stageFlags = shader->UniformSets[i].UniformBindings[j].stage;
								layout.pImmutableSamplers = nullptr;

								tex->setUniformLayoutBinding = layout;

								tex->poolSize.type = shader->UniformSets[i].UniformBindings[j].Type;
								tex->poolSize.descriptorCount = shader->UniformSets[i].UniformBindings[j].dataArrayLength;

								tTextures.push_back(tex);
								GroupTextures.push_back(tex);
							}
						
					}
	
				}

				pipe->CreateUniformDescriptorSetLayout(GroupBuffer, GroupTextures, i);

				sizeTexPerSet.push_back(texSize);
				sizePerSet.push_back(bufferSize);
				
			}
			
		}

		totalSet = i;

		pipe->uniformBuffers = tBuffers;
		pipe->textures = tTextures;

	}

	std::vector<Buffer> CreateIndexbuffer(Device device,
		Shader* shader,
		DeviceAllocator* allocator) {
		std::vector<Buffer> tBuffers;

		for (uint32_t i = 0; i < 25; i++) {
			if (shader->IndexInput.IndexDatas[i] == nullptr) {
				break;
			}
			else {

				Buffer buff;
				CreateBuffer(buff, device, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
					VK_BUFFER_USAGE_INDEX_BUFFER_BIT, shader->IndexInput.IndexSize[i],
					true, allocator);

				buff.directData = shader->IndexInput.IndexDatas[i];
				buff.transferSize = shader->IndexInput.IndexSize[i];

				tBuffers.push_back(buff);
			}
		}

		return tBuffers;
	}

	void CreateGraphicPipeline(GraphicPipeline* pipe,
		DeviceAllocator* allocator,
		Shader* shader,
		Device device,
		Swapchain swapChain) {

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

		auto& lambdaFunc = [&](VkPhysicalDevice device,
			std::vector<VkFormat> candidates, VkImageTiling imageTiling,
			VkFormatFeatureFlags flags) -> VkFormat {

			for (VkFormat format : candidates) {
				VkFormatProperties prop{};
				vkGetPhysicalDeviceFormatProperties(device, format, &prop);

				if (imageTiling == VK_IMAGE_TILING_LINEAR && (prop.linearTilingFeatures & flags) == flags) {
					return format;
				}
				else {
					if (imageTiling == VK_IMAGE_TILING_OPTIMAL &&
						(prop.optimalTilingFeatures & flags) == flags) {
						return format;
					}
				}
			}

			return VK_FORMAT_UNDEFINED;
		};

		std::vector<VkFormat> format =
		{ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT };

		VkFormat depthFormat = lambdaFunc(device.physicalDevice, format,
			VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	
		VkAttachmentDescription depthAttachment = {};
		depthAttachment.format = depthFormat;
		depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		
		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = swapChain.format.format;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkSubpassDependency subpassDepency = {};

		subpassDepency.srcSubpass = VK_SUBPASS_EXTERNAL;
		subpassDepency.dstSubpass = 0;
		subpassDepency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpassDepency.srcAccessMask = 0;
		subpassDepency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpassDepency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkAttachmentReference depthRef = {};
		depthRef.attachment = 1;
		depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription des = {};
		des.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		des.colorAttachmentCount = 1;
		des.pColorAttachments = &colorAttachmentRef;
		des.pDepthStencilAttachment = &depthRef;

		VkAttachmentDescription refs[2] = 
		{ colorAttachment , depthAttachment};

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 2;
		renderPassInfo.pAttachments = refs;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &des;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &subpassDepency;

		VkResult res2 = vkCreateRenderPass(device.logicalDevice, &renderPassInfo, nullptr, &pipe->mRenderPass);

		FRS_S_ASSERT(res2 != VK_SUCCESS);

		VkPipelineShaderStageCreateInfo vShaderStage = {};

		vShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vShaderStage.module = shader->GetVertexModule();
		vShaderStage.pName = "main";
		vShaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;

		VkPipelineShaderStageCreateInfo fShaderStage = {};

		fShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fShaderStage.module = shader->GetFragmentModule();
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
				std::vector<Texture*> textures;

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

		VkPipelineDepthStencilStateCreateInfo depthInf = {};
		depthInf.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthInf.depthTestEnable = VK_TRUE;
		depthInf.depthWriteEnable = VK_TRUE;
		//If object have depth value less then other, depth testing will be enabled
		depthInf.depthCompareOp = VK_COMPARE_OP_LESS;
		depthInf.depthBoundsTestEnable = VK_FALSE;
		depthInf.stencilTestEnable = VK_FALSE;

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
		pipelineInfo.pDepthStencilState = &depthInf;
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

		for (int i = 0; i < pipe->textures.size(); i++) {
			FinallyDestroyTexture(pipe->textures[i]);
		}

		pipe->uniformDesLayouts.resize(0);
		pipe->uniformPoolSizes.resize(0);

		pipe->desBufferHandle.clear();
		pipe->uniformWriteDescriptorSets.clear();

		for (auto& buffer : pipe->realBuffers) {
			DestroyBuffer(pipe->device, &buffer);
		}
	}

	void GraphicPipeline::GenerateWriteDescriptor(std::vector<Buffer> buffers,
		std::vector<Texture*> texs, uint32_t set) {

		std::vector<VkWriteDescriptorSet> writeSets;
		std::vector<VkDescriptorPoolSize> uniformPoolSize;

		for (uint32_t i = 0; i < buffers.size(); i++) {
			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = buffers[i].buffer;

			std::vector<VkDescriptorBufferInfo> bufferInfos;

			for (uint32_t j = 0; j < buffers[i].length; j++) {
				bufferInfo.offset = buffers[i].offset[j];
				bufferInfo.range = buffers[i].range[j];

				bufferInfos.push_back(bufferInfo);
			}

			desBufferHandle.push_back(bufferInfos);

			VkWriteDescriptorSet descriptorSet{};
			descriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorSet.descriptorCount = bufferInfos.size();
			descriptorSet.pBufferInfo = desBufferHandle[desBufferHandle.size() - 1].data();
			descriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorSet.dstBinding = buffers[i].setLayoutBinding.binding;
			descriptorSet.dstArrayElement = 0;

			writeSets.push_back(descriptorSet);

			uniformPoolSize.push_back(buffers[i].setPoolSize);
		}

		for (uint32_t i = 0; i < texs.size(); i++) {
			VkDescriptorImageInfo imageInfo = {};

			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = texs[i]->GetImageViews();

			std::vector<VkDescriptorImageInfo> imageInfos;
			std::vector<VkSampler> samplers = texs[i]->GetSamplers();
			for (uint32_t j = 0; j < 3; j++) {
				imageInfo.sampler = texs[i]->GetSamplers()[j];
				imageInfos.push_back(imageInfo);
			}

			desImageHandle.push_back(imageInfos);

			VkWriteDescriptorSet descriptorSet{};
			descriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorSet.descriptorCount = 3;
			descriptorSet.pImageInfo = desImageHandle[desImageHandle.size() - 1].data();
			descriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorSet.dstBinding = texs[i]->setUniformLayoutBinding.binding;
			descriptorSet.dstArrayElement = 0;

			writeSets.push_back(descriptorSet);

			uniformPoolSize.push_back(texs[i]->poolSize);
		}

		uniformWriteDescriptorSets.push_back(writeSets);
		uniformPoolSizes.push_back(uniformPoolSize);
	}
	
	void GraphicPipeline::CreateUniformDescriptorSetLayout(
		std::vector<Buffer> buffers,
		/*uint32_t texSize, Texture textures[],*/
		std::vector<Texture*> texs,
		uint32_t setNumber) {

		std::vector<VkDescriptorSetLayoutBinding> bindings;

		for (uint32_t i = 0; i < buffers.size(); i++) {
			bindings.push_back(buffers[i].setLayoutBinding);
		}

		for (uint32_t i = 0; i < texs.size(); i++) {
			bindings.push_back(texs[i]->setUniformLayoutBinding);
		}

		VkDescriptorSetLayoutCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		createInfo.bindingCount = bindings.size();
		createInfo.pBindings = bindings.data();

		VkDescriptorSetLayout uniformDesLayout;

		VkResult res = vkCreateDescriptorSetLayout(device.logicalDevice,
			&createInfo, nullptr, &uniformDesLayout);

		uniformDesLayouts.push_back(uniformDesLayout);

	}
}