#include "Commander.h"

namespace FRS {

	Commander::Commander(Swapchain swapChain,
		GraphicPipeline pipeline,
		Device device,
		DeviceAllocator allocator) {

		this->device = device;
		this->pipe = pipeline;
		this->chain = swapChain;
		this->bufferIndex = this->transferIndex = 0;

		vkGetDeviceQueue(device.logicalDevice, device.GetGraphicFamily(), 0, &graphicQueue);
		vkGetDeviceQueue(device.logicalDevice, device.GetPresentFamily(), 0, &presentQueue);

		buffers = Framebuffers(swapChain, pipeline, device);

		if (commandBuffer.size() > 0) {
			vkFreeCommandBuffers(device.logicalDevice,
				commandPool, commandBuffer.size(),
				commandBuffer.data());
		}

		commandBuffer.resize(buffers.frameBuffer.size(), VK_NULL_HANDLE);

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


		allocInfo.commandBufferCount = 1;

		VkResult allocRes2 = vkAllocateCommandBuffers(device.logicalDevice,
			&allocInfo,
			&transfererBuffer);

		FRS_S_ASSERT(allocRes2 != VK_SUCCESS);

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

		realBuffers = pipe.GetBuffers();

		transferBuffers.resize(realBuffers.size());

		for (uint32_t i = 0; i < realBuffers.size(); i++) {
			CreateBuffer(transferBuffers[i], device,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				realBuffers[i].getSize(),
				false, allocator);
		}

	}

	void Commander::Clear(uint16_t r,
		uint16_t g,
		uint16_t b,
		uint16_t a) {

		this->clearColor = {(float)r, (float)g, (float)b, (float)a };

	}


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

		FRS_ASSERT(res != VK_SUCCESS,"Problem!");
	
	}

	void Commander::Wait() {

		
		vkQueueWaitIdle(presentQueue);

	}

}