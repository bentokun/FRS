#include "Commander.h"

namespace FRS {

#pragma region CREATE_STAGE
	Commander::Commander(Swapchain swapChain,
		GraphicPipeline pipeline,
		Device device,
		DeviceAllocator* allocator):
	allocator(allocator){

		if (commandPool != VK_NULL_HANDLE) {

			if (commandBuffer.size() > 0) {
				vkFreeCommandBuffers(device.logicalDevice,
					commandPool, commandBuffer.size(),
					commandBuffer.data());
			}

			if (transfererCommandBuffer.size() > 0) {
				vkFreeCommandBuffers(device.logicalDevice,
					commandPool, transfererCommandBuffer.size(),
					transfererCommandBuffer.data());
			}
			if (indexCommandBuffer.size() > 0) {
				vkFreeCommandBuffers(device.logicalDevice,
					commandPool, indexCommandBuffer.size(),
					indexCommandBuffer.data());
			}

			DestroyCommander(this);
		}

		this->device = device;
		this->pipe = pipeline;
		this->chain = swapChain;
		this->bufferIndex = this->transferIndex = this->indexIndices = 0;
		this->uniformWriteDescriptorSet = this->pipe.GetWriteDescriptorLayout();
		this->uniformDescriptorPoolSize = this->pipe.GetDescriptorPoolSize();
		this->desSetLayouts = this->pipe.GetUniformDescriptorSetLayout();

		staticBuffers = pipe.GetStaticBuffer();
		uniformBuffers = pipe.GetUniformBuffer();
		indexBuffers = pipe.GetIndexBuffer();
		realTextures = pipe.GetTextures();

		staticTransferBuffers.resize(staticBuffers.size());
		uniformTransferBuffers.resize(uniformBuffers.size());
		indexTransferBuffers.resize(indexBuffers.size());
		texTransferBuffers.resize(realTextures.size());

		for (uint32_t i = 0; i < staticBuffers.size(); i++) {

			CreateBuffer(staticTransferBuffers[i], device,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				staticBuffers[i].getSize(),
				false, allocator);

		}

		for (uint32_t i = 0; i < uniformBuffers.size(); i++) {

			CreateBuffer(uniformTransferBuffers[i], device,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				uniformBuffers[i].getSize(),
				false, allocator);

		}

		for (uint32_t i = 0; i < indexBuffers.size(); i++) {

			CreateBuffer(indexTransferBuffers[i], device,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				indexBuffers[i].getSize(),
				false, allocator);

		}

		for (uint32_t i = 0; i < realTextures.size(); i++) {
			CreateBuffer(texTransferBuffers[i], device,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				realTextures[i].GetSize(),
				false, allocator);
		}

		vkGetDeviceQueue(device.logicalDevice, device.GetGraphicFamily(), 0, &graphicQueue);
		vkGetDeviceQueue(device.logicalDevice, device.GetPresentFamily(), 0, &presentQueue);

		buffers = Framebuffers(swapChain, pipeline, device);

		indexCommandBuffer.resize(indexBuffers.size(), VK_NULL_HANDLE);
		commandBuffer.resize(buffers.frameBuffer.size(), VK_NULL_HANDLE);
		transfererCommandBuffer.resize(staticBuffers.size(), VK_NULL_HANDLE);
		textureTransfererCmdBuff.resize(realTextures.size(), VK_NULL_HANDLE);

		VkCommandPoolCreateInfo poolInfo = {};

		//Not in the loop.
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = device.GetGraphicFamily();
		poolInfo.flags = 0;

		VkResult poolResult = vkCreateCommandPool(device.logicalDevice,
			&poolInfo, nullptr, &commandPool);

		if (poolResult != VK_SUCCESS) {
			throw std::runtime_error("Lol");
		}

		VkCommandBufferAllocateInfo allocInfo = {};

		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)(buffers.frameBuffer.size());

		VkResult allocRes = vkAllocateCommandBuffers(device.logicalDevice,
			&allocInfo,
			commandBuffer.data());

		FRS_ASSERT_WV(allocRes != VK_SUCCESS, "Cant alloc command buffer!", allocRes,
			0);

		allocInfo.commandBufferCount = staticBuffers.size();

		VkResult allocRes2 = vkAllocateCommandBuffers(device.logicalDevice,
			&allocInfo,
			transfererCommandBuffer.data());

		FRS_S_ASSERT(allocRes2 != VK_SUCCESS);

		allocInfo.commandBufferCount = indexBuffers.size();
		VkResult allocRes3 = vkAllocateCommandBuffers(device.logicalDevice,
			&allocInfo,
			indexCommandBuffer.data());

		FRS_S_ASSERT(allocRes3 != VK_SUCCESS);

		allocInfo.commandBufferCount = realTextures.size();
		VkResult allocRes4 = vkAllocateCommandBuffers(device.logicalDevice,
			&allocInfo,
			textureTransfererCmdBuff.data());

		FRS_S_ASSERT(allocRes4 != VK_SUCCESS);

		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

		VkResult res1 = vkCreateSemaphore(device.logicalDevice,
			&semaphoreInfo, nullptr, &imageSemaphore);

		VkResult res2 = vkCreateSemaphore(device.logicalDevice,
			&semaphoreInfo, nullptr, &rendererSemaphore);

		VkResult res3 = vkCreateFence(device.logicalDevice,
			&fenceInfo, nullptr, &dataTransferFence);

		FRS_S_ASSERT(res1 != VK_SUCCESS);
		FRS_S_ASSERT(res2 != VK_SUCCESS);
		FRS_S_ASSERT(res3 != VK_SUCCESS);

		CreateUniformDescriptorSets();

		for (auto& buffer : staticBuffers) {

			if (buffer.directData == nullptr) {
				throw std::runtime_error("Pass data pointer may fail");
			}

			SetStaticData(buffer,
				0, buffer.transferSize, buffer.dataPointer);
		}


		for (auto& buffer : indexBuffers) {

			if (buffer.directData == nullptr) {
				throw std::runtime_error("Pass data pointer may fail");
			}

			SetIndexData(buffer, buffer.GetBlock().offSet,
				buffer.transferSize, buffer.directData);
		}


	}


	void CreateCommander(Commander* commander, 
		Swapchain swapChain,
		GraphicPipeline pipeline,
		Device device,
		DeviceAllocator* allocator) {

		bool haveDestroy = false;

		if (commander->commandPool != VK_NULL_HANDLE) {

			if (commander->commandBuffer.size() > 0) {
				vkFreeCommandBuffers(device.logicalDevice,
					commander->commandPool, commander->commandBuffer.size(),
					commander->commandBuffer.data());
			}

			if (commander->transfererCommandBuffer.size() > 0) {
				vkFreeCommandBuffers(device.logicalDevice,
					commander->commandPool, commander->transfererCommandBuffer.size(),
					commander->transfererCommandBuffer.data());
			}

			if (commander->indexCommandBuffer.size() > 0) {
				vkFreeCommandBuffers(device.logicalDevice,
					commander->commandPool, commander->indexCommandBuffer.size(),
					commander->indexCommandBuffer.data());
			}

			DestroyCommander(commander);
		}

		commander->allocator = allocator;

		commander->device = device;
		commander->pipe = pipeline;
		commander->chain = swapChain;
		commander->bufferIndex = commander->transferIndex = commander->indexIndices = 0;
		commander->desSetLayouts = commander->pipe.GetUniformDescriptorSetLayout();

		commander->staticBuffers = commander->pipe.GetStaticBuffer();
		commander->uniformBuffers = commander->pipe.GetUniformBuffer();
		commander->indexBuffers = commander->pipe.GetIndexBuffer();
		commander->realTextures = commander->pipe.GetTextures();

		commander->staticTransferBuffers.resize(commander->staticBuffers.size());
		commander->uniformTransferBuffers.resize(commander->uniformBuffers.size());
		commander->indexTransferBuffers.resize(commander->indexBuffers.size());
		commander->texTransferBuffers.resize(commander->realTextures.size());
		commander->textureTransfererCmdBuff.resize(commander->realTextures.size());

		for (uint32_t i = 0; i <commander->staticBuffers.size(); i++) {

			CreateBuffer(commander->staticTransferBuffers[i], device,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				commander->staticBuffers[i].getSize(),
				false, allocator);

		}

		for (uint32_t i = 0; i < commander->uniformBuffers.size(); i++) {

			CreateBuffer(commander->uniformTransferBuffers[i], device,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				commander->uniformBuffers[i].getSize(),
				false, allocator);

		}

		for (uint32_t i = 0; i < commander->indexBuffers.size(); i++) {

			CreateBuffer(commander->indexTransferBuffers[i], device,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				commander->indexBuffers[i].getSize(),
				false, allocator);

		}

		for (uint32_t i = 0; i < commander->realTextures.size(); i++) {
			CreateBuffer(commander->texTransferBuffers[i], device,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				commander->realTextures[i].GetSize(),
				false, allocator);
		}

		vkGetDeviceQueue(device.logicalDevice, device.GetGraphicFamily(), 0, &commander->graphicQueue);
		vkGetDeviceQueue(device.logicalDevice, device.GetPresentFamily(), 0, &commander->presentQueue);

		commander->buffers = Framebuffers(swapChain, pipeline, device);

		commander->commandBuffer.clear();
		commander->transfererCommandBuffer.clear();
		commander->indexCommandBuffer.clear();

		commander->commandBuffer.resize(commander->buffers.frameBuffer.size(), VK_NULL_HANDLE);
		commander->transfererCommandBuffer.resize(commander->staticBuffers.size(), VK_NULL_HANDLE);
		commander->indexCommandBuffer.resize(commander->indexBuffers.size(), VK_NULL_HANDLE);
		commander->textureTransfererCmdBuff.resize(commander->realTextures.size(), VK_NULL_HANDLE);

		VkCommandPoolCreateInfo poolInfo = {};

		//Not in the loop.
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = device.GetGraphicFamily();
		poolInfo.flags = 0;

		VkResult poolResult = vkCreateCommandPool(device.logicalDevice,
			&poolInfo, nullptr, &commander->commandPool);

		if (poolResult != VK_SUCCESS) {
			throw std::runtime_error("Lol");
		}

		VkCommandBufferAllocateInfo allocInfo = {};

		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commander->commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)(commander->buffers.frameBuffer.size());

		VkResult allocRes = vkAllocateCommandBuffers(device.logicalDevice,
			&allocInfo,
			commander->commandBuffer.data());

		FRS_ASSERT_WV(allocRes != VK_SUCCESS, "Cant alloc command buffer!", allocRes,
			0);

		if (commander->staticBuffers.size() > 0) {
			allocInfo.commandBufferCount = commander->staticBuffers.size();

			VkResult allocRes2 = vkAllocateCommandBuffers(device.logicalDevice,
				&allocInfo,
				commander->transfererCommandBuffer.data());

			FRS_S_ASSERT(allocRes2 != VK_SUCCESS);
		}
	
		if (commander->indexBuffers.size() > 0) {
			allocInfo.commandBufferCount = commander->indexBuffers.size();
			VkResult allocRes3 = vkAllocateCommandBuffers(device.logicalDevice,
				&allocInfo,
				commander->indexCommandBuffer.data());

			FRS_S_ASSERT(allocRes3 != VK_SUCCESS);
		}
		

		if (commander->realTextures.size() > 0) {
			allocInfo.commandBufferCount = commander->realTextures.size();
			VkResult allocRes4 = vkAllocateCommandBuffers(device.logicalDevice,
				&allocInfo,
				commander->textureTransfererCmdBuff.data());

			FRS_S_ASSERT(allocRes4 != VK_SUCCESS);
		}
		


		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

		VkResult res1 = vkCreateSemaphore(device.logicalDevice,
			&semaphoreInfo, nullptr, &commander->imageSemaphore);

		VkResult res2 = vkCreateSemaphore(device.logicalDevice,
			&semaphoreInfo, nullptr, &commander->rendererSemaphore);

		VkResult res3 = vkCreateFence(device.logicalDevice,
			&fenceInfo, nullptr, &commander->dataTransferFence);

		FRS_S_ASSERT(res1 != VK_SUCCESS);
		FRS_S_ASSERT(res2 != VK_SUCCESS);
		FRS_S_ASSERT(res3 != VK_SUCCESS);
		
		for (auto& buffer : commander->staticBuffers) {

			if (buffer.directData == nullptr) {
				throw std::runtime_error("Pass data pointer may fail");
			}

			commander->SetStaticData(buffer,
				0, buffer.transferSize, buffer.directData);
		}

		for (auto& buffer : commander->indexBuffers) {

			if (buffer.directData == nullptr) {
				throw std::runtime_error("Pass data pointer may fail");
			}

			commander->SetIndexData(buffer, 0,
				buffer.transferSize, buffer.directData);
		}

		for (auto& tex : commander->realTextures) {

			if (tex.GetImage() == nullptr) {
				throw std::runtime_error("Pass data pointer may fail");
			}

			commander->SetStaticData(tex, 0);
			tex.CreateSampler();

		}

		for (uint32_t i = 0; i < commander->pipe.totalSet; i++) {
			std::vector<Buffer> gBuffers;
			std::vector<Texture> gTextures;
			
			//They won't be messed up, since they were push_back in order
			for (uint32_t j = 0; j < commander->pipe.sizePerSet[i]; j++) {
				gBuffers.push_back(commander->uniformBuffers[j]);
			}

			for (uint32_t j = 0; j < commander->pipe.sizeTexPerSet[i]; j++) {
				gTextures.push_back(commander->realTextures[j]);

			}

			commander->pipe.GenerateWriteDescriptor(gBuffers, gTextures, i);
		}

		commander->uniformWriteDescriptorSet = commander->pipe.GetWriteDescriptorLayout();
		commander->uniformDescriptorPoolSize = commander->pipe.GetDescriptorPoolSize();

		commander->CreateUniformDescriptorSets();

	}

#pragma endregion

#pragma region DATA_HANDLE
	void Commander::UpdateData(Shader shader) {
	
		for (uint32_t i = 0; i < pipe.totalSet; i++) {
			for (uint32_t j = 0; j < pipe.sizePerSet[i]; j++) {
				
				SetSimulatousData(uniformBuffers[(i+1)*j],
					0, shader.UniformSets[i].BindingSize[j],
					shader.UniformSets[i].BindingDatas[j]);

			}
		}
	}

	void Commander::SubmitData(std::vector<VkCommandBuffer> buffers) {

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = buffers.size();
		submitInfo.pCommandBuffers = buffers.data();

		vkResetFences(device.logicalDevice, 1, &dataTransferFence);

		VkResult result = vkQueueSubmit(graphicQueue,
			1, &submitInfo, dataTransferFence);

		FRS_S_ASSERT(result != VK_SUCCESS);

	}

	void Commander::WaitData() {

		vkQueueWaitIdle(graphicQueue);

		vkWaitForFences(device.logicalDevice,
			1, &dataTransferFence,
			true,
			(uint64_t)std::numeric_limits<uint64_t>::max);

		vkResetFences(device.logicalDevice, 1, &dataTransferFence);

		bufferIndex = 0;
	}

	void Commander::LayoutImage(Texture para,VkImageLayout oldLayout,
		VkImageLayout newLayout, uint32_t baseLevel, uint32_t levelCount, VkPipelineStageFlags srcStage,
		VkPipelineStageFlags dstStage) {

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		VkCommandBuffer commandBuffer;

		VkCommandBufferAllocateInfo allocateInfo = {};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.commandPool = commandPool;
		allocateInfo.commandBufferCount = 1;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		vkAllocateCommandBuffers(device.logicalDevice,
			&allocateInfo, &commandBuffer);

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = para.GetVkImage();
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = baseLevel;
		barrier.subresourceRange.levelCount = levelCount;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		switch (oldLayout)
		{
			case VK_IMAGE_LAYOUT_UNDEFINED:
				barrier.srcAccessMask = 0;
				break;

			case VK_IMAGE_LAYOUT_PREINITIALIZED:
				barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
				barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
				barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				break;

			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				break;

			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
				barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
				break;

			default:
				break;
		}

		switch (newLayout)
		{
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			barrier.dstAccessMask = barrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			if (barrier.srcAccessMask == 0)
			{
				barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
			}
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		default:
			break;
		}

		vkCmdPipelineBarrier(commandBuffer,
			srcStage, dstStage,
			0, 0, nullptr, 0, nullptr, 1, &barrier);

		vkEndCommandBuffer(commandBuffer);
		
		std::vector<VkCommandBuffer> commands;
		commands.push_back(commandBuffer);

		SubmitData(commands);

		vkQueueWaitIdle(graphicQueue);

		vkFreeCommandBuffers(device.logicalDevice, commandPool, 1, &commandBuffer);

	}

	void Commander::SetData(Buffer src, Texture des, uint32_t offsetSrc, uint32_t offsetDst) {

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(textureTransfererCmdBuff[texTransIndex], &beginInfo);
		
		VkBufferImageCopy copyRegion{};
		std::vector<VkBufferImageCopy> copyRegions;

		uint32_t offset = 0;

		for (uint32_t i = 0; i < des.GetMipLevel(); i++) {
			copyRegion.imageSubresource.mipLevel = i;
			copyRegion.imageExtent.width = des.tex[i].extent().x;
			copyRegion.imageExtent.height = des.tex[i].extent().y; 
			copyRegion.imageSubresource.layerCount = 1;
			copyRegion.imageSubresource.baseArrayLayer = 0;
			copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copyRegion.bufferOffset = offset;
			copyRegion.imageExtent.depth = 1;

			copyRegions.push_back(copyRegion);
			
			offset += des.tex[i].size();

		}

		LayoutImage(des, VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0,
			des.GetMipLevel(),
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

		vkCmdCopyBufferToImage(textureTransfererCmdBuff[texTransIndex],
			src.buffer, des.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			copyRegions.size(), copyRegions.data());

		vkEndCommandBuffer(textureTransfererCmdBuff[texTransIndex]);
		
	}

	
	void Commander::SetStaticData(Texture tex, VkDeviceSize offset) {

		uint32_t index;

		texTransIndex = 0;

		for (uint32_t i = 0; i < realTextures.size(); i++) {

			if (realTextures[i] == tex) {
				index = i;
				break;
			}
		}

	
		memcpy(texTransferBuffers[index].GetBlock().ptr, 
			tex.tex.data(), tex.GetSize());
	
		SetData(texTransferBuffers[index], tex, 0,
			tex.GetBlock().offSet);

		std::vector<VkCommandBuffer> commandBufferz;
		commandBufferz.push_back(textureTransfererCmdBuff[index]);
		
		SubmitData(commandBufferz);
		WaitData();

		LayoutImage(tex, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0,
			tex.GetMipLevel(),
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

	}

	void Commander::SetStaticData(Buffer buffer, VkDeviceSize offset,
		VkDeviceSize size, void* data) {

		uint32_t index;

		bool isUniformBuffer = ((buffer.getUsage() | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) == buffer.getUsage());

		if (isUniformBuffer) {
			return;
		}

		for (uint32_t i = 0; i < staticBuffers.size(); i++) {

			if (staticBuffers[i] == buffer) {

				index = i;
				break;

			}
		}

		memcpy((staticTransferBuffers)[index].GetBlock().ptr, data, size);
		SetData((staticTransferBuffers)[index], buffer, 0, offset, size);

		if (bufferIndex == staticBuffers.size() - 1) {
			SubmitData(transfererCommandBuffer);
			WaitData();

		}

		bufferIndex++;
	}

	void Commander::SetIndexData(Buffer buffer, VkDeviceSize offset,
		VkDeviceSize size, void* data) {

		uint32_t index;

		bool isUniformBuffer = ((buffer.getUsage() | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) == buffer.getUsage());

		if (isUniformBuffer) {
			return;
		}

		for (uint32_t i = 0; i < indexBuffers.size(); i++) {

			if (indexBuffers[i] == buffer) {

				index = i;
				break;

			}
		}

		memcpy((indexTransferBuffers)[index].GetBlock().ptr, 
			data, size);
		
		SetDataWithIndex((indexTransferBuffers)[index], 
			buffer,
			0, offset, size);

		if (indexIndices == indexBuffers.size() - 1) {
			SubmitData(indexCommandBuffer);
			WaitData();

		}

		indexIndices++;
	}

	void Commander::SetDataWithIndex(const Buffer& src,
		Buffer& dst,
		VkDeviceSize offsetSrc,
		VkDeviceSize offsetDst,
		VkDeviceSize size) {

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(indexCommandBuffer[indexIndices], &beginInfo);

		VkBufferCopy copyRegion = {};
		copyRegion.srcOffset = offsetSrc;
		copyRegion.dstOffset = offsetDst;
		copyRegion.size = size;

		VkBufferMemoryBarrier barrier = {};
		barrier.offset = offsetSrc;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.buffer = dst.buffer;
		barrier.size = size;
		barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;

		vkCmdCopyBuffer(indexCommandBuffer[indexIndices], src.buffer, dst.buffer, 1, &copyRegion);
		vkCmdPipelineBarrier(indexCommandBuffer[indexIndices],
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			0, 0, nullptr, 1, &barrier, 0, nullptr);

		vkEndCommandBuffer(indexCommandBuffer[indexIndices]);


	}
	void Commander::SetSimulatousData(Buffer& dst,
		VkDeviceSize offset,
		VkDeviceSize size,
		void* data) {

		uint32_t index;

		bool isUniformBuffer = ((dst.getUsage() | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) == dst.getUsage());

		if (!isUniformBuffer) {
			return;
		}

		for (uint32_t i = 0; i < uniformBuffers.size(); i++) {

			if (uniformBuffers[i] == dst) {
				index = i;
			}
		}

		memcpy((uniformTransferBuffers)[index].GetBlock().ptr, data, size);
	

		VkCommandBufferAllocateInfo bufferAllocate = {};
		VkCommandBuffer commandBuffer = VK_NULL_HANDLE;

		bufferAllocate.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		bufferAllocate.commandBufferCount = 1;
		bufferAllocate.commandPool = commandPool;
		
		vkAllocateCommandBuffers(device.logicalDevice, &bufferAllocate,
			&commandBuffer);

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		VkBufferCopy copyRegion = {};
		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = offset;
		copyRegion.size = size;

		VkBufferMemoryBarrier barrier = {};
		barrier.offset = offset;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.buffer = dst.buffer;
		barrier.size = size;
		barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;


		vkCmdCopyBuffer(commandBuffer, uniformTransferBuffers[index].buffer, dst.buffer, 1, &copyRegion);
		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			0, 0, nullptr, 1, &barrier, 0, nullptr);


		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkResetFences(device.logicalDevice, 1, &dataTransferFence);
		vkQueueSubmit(graphicQueue, 1, &submitInfo, dataTransferFence);

		vkQueueWaitIdle(graphicQueue);
		vkWaitForFences(device.logicalDevice, 1, &dataTransferFence,
			true, (uint64_t)std::numeric_limits<uint64_t>::max);
		vkResetFences(device.logicalDevice, 1, &dataTransferFence);

		vkFreeCommandBuffers(device.logicalDevice, commandPool, 1, &commandBuffer);

	}

	void Commander::SetData(const Buffer& src, Buffer& dst,
		VkDeviceSize offsetSrc,
		VkDeviceSize offsetDst,
		VkDeviceSize size) {

		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		
		vkBeginCommandBuffer(transfererCommandBuffer[bufferIndex], &beginInfo);

		VkBufferCopy copyRegion = {};
		copyRegion.srcOffset = offsetSrc;
		copyRegion.dstOffset = offsetDst;
		copyRegion.size = size;

		VkBufferMemoryBarrier barrier = {};
		barrier.offset = offsetSrc;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.buffer = dst.buffer;
		barrier.size = size;
		barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;


		vkCmdCopyBuffer(transfererCommandBuffer[bufferIndex], src.buffer, dst.buffer, 1, &copyRegion);
		vkCmdPipelineBarrier(transfererCommandBuffer[bufferIndex],
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			0, 0, nullptr, 1, &barrier, 0, nullptr);


		vkEndCommandBuffer(transfererCommandBuffer[bufferIndex]);


	}

#pragma endregion

#pragma region DESTROYER

	void DestroyCommander(Commander* commander) {

		vkQueueWaitIdle(commander->graphicQueue);

		for (int i = 0; i < commander->uniformTransferBuffers.size(); i++) {
			DestroyBuffer(commander->device, &commander->uniformTransferBuffers[i]);
		}

		for (int i = 0; i < commander->staticTransferBuffers.size(); i++) {
			DestroyBuffer(commander->device, &commander->staticTransferBuffers[i]);
		}

		for (int i = 0; i < commander->indexTransferBuffers.size(); i++) {
			DestroyBuffer(commander->device, &commander->indexTransferBuffers[i]);
		}

		for (int i = 0; i < commander->texTransferBuffers.size(); i++) {
			DestroyBuffer(commander->device, &commander->texTransferBuffers[i]);
		}

		for (int i = 0; i < commander->realTextures.size(); i++) {
			ParticallyDestroyTexture(commander->realTextures[i]);
		}

		commander->uniformTransferBuffers.resize(0);
		commander->staticTransferBuffers.resize(0);
		commander->indexTransferBuffers.resize(0);
		commander->texTransferBuffers.resize(0);

		vkDestroyFence(commander->device.logicalDevice, commander->dataTransferFence,
			nullptr);

		vkDestroySemaphore(commander->device.logicalDevice, commander->imageSemaphore, nullptr);
		vkDestroySemaphore(commander->device.logicalDevice, commander->rendererSemaphore, nullptr);

		vkDestroyCommandPool(commander->device.logicalDevice, commander->commandPool, nullptr);
		vkDestroyDescriptorPool(commander->device.logicalDevice, commander->descriptorPool, nullptr);

		commander->commandPool = VK_NULL_HANDLE;
		commander->uniformDescriptorSets.resize(0, VK_NULL_HANDLE);
		commander->uniformDescriptorPoolSize.clear();
		commander->uniformDescriptorPoolSize.resize(0);

		DestroyFramebuffer(&commander->buffers);
	}

#pragma endregion

#pragma region VK_CMD_WRAPPER
	void Commander::Draw(FRSint vertexCount,
		FRSint instance,
		FRSint firstVertex,
		FRSint firstInstance) {

		vkCmdDraw(commandBuffer[currentBuffer],
			vertexCount, instance, firstVertex,
			firstInstance);
	}

	void Commander::BindVertexBuffers(uint32_t firstBinding,
		uint32_t sizeBinding,
		Buffer buffers[],
		VkDeviceSize offset[]) {

		std::vector<VkBuffer> tbuffers;

		for (uint32_t i = 0; i < sizeBinding; i++) {
			tbuffers.push_back(buffers[i].buffer);
		}

		vkCmdBindVertexBuffers(commandBuffer[currentBuffer],
			firstBinding, sizeBinding, tbuffers.data(), offset);
	}

	void Commander::BindIndexBuffers(Buffer buffer, VkDeviceSize offset,
		VkIndexType index) {

		vkCmdBindIndexBuffer(commandBuffer[currentBuffer],
			buffer.buffer, offset, index);

	}

	void Commander::DrawIndexed(uint32_t indexCount,
		uint32_t instanceCount,
		uint32_t firstIndex,
		uint32_t vertexOffset,
		uint32_t firstInstance) {

		vkCmdDrawIndexed(commandBuffer[currentBuffer], indexCount,
			instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	void Commander::Clear(uint16_t r,
		uint16_t g,
		uint16_t b,
		uint16_t a) {

		this->clearColor = { (float)r, (float)g, (float)b, (float)a };

	}

	void Commander::UpdateDescriptorSet(uint32_t numberDesWrite, VkWriteDescriptorSet* writer,
		uint32_t numberDesCopier, VkCopyDescriptorSet* copier) {

		vkUpdateDescriptorSets(device.logicalDevice,
			numberDesWrite, writer, numberDesCopier, copier);

	}

	void Commander::BindDescriptorSet(VkPipelineBindPoint bindPoint,
		uint32_t dynamicOffsetCount, uint32_t* dynamicOffset) {

		vkCmdBindDescriptorSets(commandBuffer[currentBuffer], bindPoint,
			pipe.mLayout, 0, uniformDescriptorSets.size(), uniformDescriptorSets.data(),
			dynamicOffsetCount, dynamicOffset);
	}

#pragma endregion

#pragma region RENDERING
	void Commander::ReadDrawingCommand(std::function<void()> drawingFunc) {

		FRS_ASSERT(drawingFunc == nullptr,
			"You havent set the drawing Func for commander to draw");

		for (size_t i = 0; i < commandBuffer.size(); i++) {
			currentBuffer = i;
			drawingFunc();
		}

	}

	void Commander::Start() {

		VkCommandBufferBeginInfo bufferBegin = {};

		//Wait for last frame to finish, then conitnue
		//Not execute it right away, also us render pass
		bufferBegin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		bufferBegin.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		bufferBegin.pInheritanceInfo = nullptr;

		VkResult result = vkBeginCommandBuffer(commandBuffer[currentBuffer], &bufferBegin);

		VkRenderPassBeginInfo renderBeginInfo = {};
		renderBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderBeginInfo.renderPass = pipe.mRenderPass;
		renderBeginInfo.framebuffer = buffers.frameBuffer[currentBuffer];
		renderBeginInfo.renderArea.extent = chain.swapChainExtent;
		renderBeginInfo.renderArea.offset = { 0,0 };

		renderBeginInfo.clearValueCount = 1;
		renderBeginInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffer[currentBuffer], &renderBeginInfo,
			VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(commandBuffer[currentBuffer],
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipe.pipeline);


	}

	void Commander::End() {

		vkCmdEndRenderPass(commandBuffer[currentBuffer]);

		if (vkEndCommandBuffer(commandBuffer[currentBuffer]) != VK_SUCCESS) {

			throw std::runtime_error("Command Buffer recording state failed, check to see if you have turn on option to start recording");

		}

	}

	void Commander::Submit() {

		VkResult result = vkAcquireNextImageKHR(device.logicalDevice,
			chain.swapChain, (uint64_t)std::numeric_limits<uint64_t>::max,
			imageSemaphore, VK_NULL_HANDLE,
			&imageIndex);

		FRS_ASSERT(result != VK_SUCCESS, "cant get image");

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { imageSemaphore };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer[imageIndex];

		VkSemaphore signalSemaphores[] = { rendererSemaphore };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		if (vkQueueSubmit(graphicQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
			throw std::runtime_error("Cant create queue!");
		}
	}

	void Commander::Render() {

		VkSemaphore signalSemaphores[] = { rendererSemaphore };

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &(chain.swapChain);
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr;

		VkResult res = vkQueuePresentKHR(presentQueue, &presentInfo);

		FRS_ASSERT(res != VK_SUCCESS, "Problem!");

	}

	void Commander::Wait() {
		vkQueueWaitIdle(presentQueue);
	}
#pragma endregion


}