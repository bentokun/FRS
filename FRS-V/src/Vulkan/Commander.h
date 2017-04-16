#pragma once

#include "Rules.h"
#include "Framebuffer.h"
#include "GraphicPipeline.h"
#include "Buffer.h"

#include <limits>

#ifdef _WIN32
#ifdef FRSV_EXPORTS
#define TFSAPI __declspec(dllexport)
#else
#define TFSAPI __declspec(dllimport)
#endif
#endif

namespace FRS {

	class TFSAPI Commander{

	public:

		Commander() { };
		Commander(Swapchain swapChain,
			GraphicPipeline pipeline,
			Device device,
			DeviceAllocator allocator);

		void ReadDrawingCommand(std::function<void()> drawingFunc);

		void Start();
		void End();

		void Submit();
		void Render();
		void Wait();

		void SubmitData() {

			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &transfererBuffer;

			vkResetFences(device.logicalDevice, 1, &dataTransferFence);

			VkResult result = vkQueueSubmit(graphicQueue, 1, &submitInfo, dataTransferFence);
			
			FRS_S_ASSERT(result != VK_SUCCESS);
		
		}

		void WaitData() {

			vkQueueWaitIdle(graphicQueue);

			vkWaitForFences(device.logicalDevice, 1, &dataTransferFence,
				true, (uint64_t)std::numeric_limits<uint64_t>::max);

			vkResetFences(device.logicalDevice, 1, &dataTransferFence);

			bufferIndex = 0;
		}
		
		void SetData(Buffer buffer, VkDeviceSize offset,
			VkDeviceSize size, void* data) {

			uint32_t index;

			for (uint32_t i = 0; i < realBuffers.size(); i++) {
				if (realBuffers[i] == buffer) {
					index = i;
					break;
				}
			}

			memcpy((transferBuffers)[index].getPtr(), data, size);
			vkUnmapMemory(device.logicalDevice, transferBuffers[index].GetBlock().memory);

			SetData((transferBuffers)[index], buffer, 0, offset, size);

			bufferIndex++;
		
			if (bufferIndex == realBuffers.size()) {
				SubmitData();
				WaitData();
			}
		}

		void SetData(const Buffer& src, Buffer& dst,
			VkDeviceSize offsetSrc,
			VkDeviceSize offsetDst,
			VkDeviceSize size) {


			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

			vkBeginCommandBuffer(transfererBuffer, &beginInfo);

			VkBufferCopy copyRegion = {};
			copyRegion.srcOffset = offsetSrc;
			copyRegion.dstOffset = offsetDst;
			copyRegion.size = size;

			VkBufferMemoryBarrier barrier = { };
			barrier.offset = offsetSrc;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.buffer = dst.buffer;
			barrier.size = size;
			barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;


			vkCmdCopyBuffer(transfererBuffer, src.buffer, dst.buffer, 1, &copyRegion);
			vkCmdPipelineBarrier(transfererBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				0, 0, nullptr, 1, &barrier, 0, nullptr);


			vkEndCommandBuffer(transfererBuffer);

		}

		friend void DestroyCommander(Commander commander) {

			vkDestroyFence(commander.device.logicalDevice, commander.dataTransferFence,
				nullptr);

			vkDestroySemaphore(commander.device.logicalDevice, commander.imageSemaphore, nullptr);
			vkDestroySemaphore(commander.device.logicalDevice, commander.rendererSemaphore, nullptr);

			vkDestroyCommandPool(commander.device.logicalDevice, commander.commandPool, nullptr);
			DestroyFramebuffer(&commander.buffers);
		}

		void Draw(FRSint vertexCount,
			FRSint instance,
			FRSint firstVertex,
			FRSint firstInstance) {

			vkCmdDraw(commandBuffer[currentBuffer],
				vertexCount, instance, firstVertex,
				firstInstance);
		}

		void DrawIndexed(FRSint indexCount,
			FRSint instanceCount,
			FRSint firstIndex,
			FRSint vertexOffSet,
			FRSint firstInstance) {

			vkCmdDrawIndexed(commandBuffer[currentBuffer], indexCount,
				instanceCount, firstIndex, vertexOffSet, firstInstance);
		}

		void BindVertexBuffers(uint32_t firstBinding, 
			uint32_t sizeBinding,
			Buffer buffers[],
		    VkDeviceSize offset[]) {

			std::vector<VkBuffer> tbuffers;
			
			for (uint32_t i = 0; i < sizeBinding ;i++) {
				tbuffers.push_back(buffers[i].buffer);
			}

			vkCmdBindVertexBuffers(commandBuffer[currentBuffer],
				firstBinding, sizeBinding, tbuffers.data(), offset);
		}

		//Set the color to clear
		//Do before loop, in Draw
		void Clear(uint16_t r = 0.0f,
			uint16_t g = 0.0f,
			uint16_t b = 0.0f,
			uint16_t a = 1.0f);



		VkCommandPool& GetCommandPool() {
			return commandPool;
		}

	private:

		VkCommandPool commandPool;
		VkClearValue clearColor;
		VkSemaphore imageSemaphore, rendererSemaphore = VK_NULL_HANDLE;
		VkQueue graphicQueue, presentQueue = VK_NULL_HANDLE;
		VkFence dataTransferFence;

		uint32_t imageIndex = 0;

		std::vector<VkCommandBuffer> commandBuffer;
		VkCommandBuffer transfererBuffer;
		std::vector<Buffer> realBuffers, transferBuffers;

		uint32_t currentBuffer;
		Framebuffers buffers;
		Device device;
		GraphicPipeline pipe;
		Swapchain chain;

		uint32_t bufferIndex, transferIndex = 0;

	};


}