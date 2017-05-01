#pragma once
#pragma warning (disable: 4251 4267)

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

		friend void CreateCommander(Commander* commander, Swapchain swapChain,
			GraphicPipeline pipeline,
			Device device,
			DeviceAllocator allocator);

		void ReadDrawingCommand(std::function<void()> drawingFunc);

		void Start();
		void End();

		void Submit();
		void Render();
		void Wait();

		void SubmitData(std::vector<VkCommandBuffer> buffers);
		void WaitData();
		
		void SetStaticData(Buffer buffer, VkDeviceSize offset,
			VkDeviceSize size, void* data);

		void SetData(const Buffer& src, Buffer& dst,
			VkDeviceSize offsetSrc,
			VkDeviceSize offsetDst,
			VkDeviceSize size);

		friend void DestroyCommander(Commander* commander);

		void Draw(FRSint vertexCount,
			FRSint instance,
			FRSint firstVertex,
			FRSint firstInstance);

		void BindVertexBuffers(uint32_t firstBinding,
			uint32_t sizeBinding,
			Buffer buffers[],
		VkDeviceSize offset[]);

		void BindIndexBuffers(Buffer buffer, VkDeviceSize offset,
			VkIndexType index);

		void DrawIndexed(uint32_t indexCount,
			uint32_t instanceCount,
			uint32_t firstIndex,
			uint32_t vertexOffset,
			uint32_t firstInstance);

		void UpdateDescriptorSet(uint32_t numberDesWrite, VkWriteDescriptorSet* writer,
			uint32_t numberDesCopier, VkCopyDescriptorSet* copier);

		void BindDescriptorSet(VkPipelineBindPoint bindPoint,
			uint32_t dynamicOffsetCount, uint32_t* dynamicOffset);

		void SetIndexData(Buffer buffer, VkDeviceSize offset,
			VkDeviceSize size, void* data);


		void SetDataWithIndex(const Buffer& src, Buffer& dst,
			VkDeviceSize offsetSrc,
			VkDeviceSize offsetDst,
			VkDeviceSize size);
		//Set the color to clear
		//Do before loop, in Draw
		void Clear(uint16_t r = 0.0f,
			uint16_t g = 0.0f,
			uint16_t b = 0.0f,
			uint16_t a = 1.0f);

		VkCommandPool& GetCommandPool() {
			return commandPool;
		}

		void SetSimulatousData(Buffer& src,
			VkDeviceSize offset,
			VkDeviceSize size,
			void* data);

		void UpdateData(Shader shader);

		/*
		void SetStaticData(Texture tex, VkDeviceSize offset);
		void SetData(Texture src, Texture des, uint32_t offsetSrc, uint32_t offsetDst);
		void LayoutImage(Texture para, VkImageLayout oldLayout,
			VkImageLayout newLayout);*/

		void CreateUniformDescriptorSets() {

			VkDescriptorPoolCreateInfo poolInfo = {};

			poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			poolInfo.maxSets = desSetLayouts.size();
			poolInfo.poolSizeCount = uniformDescriptorPoolSize.data()->size();
			poolInfo.pPoolSizes = uniformDescriptorPoolSize.data()->data();
		
			VkResult res1 = vkCreateDescriptorPool(device.logicalDevice,
				&poolInfo, nullptr, &descriptorPool);

			for (uint32_t i = 0; i < desSetLayouts.size(); i++) {
			
				VkDescriptorSetAllocateInfo allocInfo = {};
				allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				allocInfo.descriptorPool = descriptorPool;

				allocInfo.descriptorSetCount = desSetLayouts.size();
				allocInfo.pSetLayouts = desSetLayouts.data();
			
				VkDescriptorSet set = {};
			
				VkResult res2 = vkAllocateDescriptorSets(device.logicalDevice,
					&allocInfo, &set);

				uniformDescriptorSets.push_back(set);
			
				for (uint32_t j = 0; j < uniformWriteDescriptorSet[i].size(); j++) {
					uniformWriteDescriptorSet[i][j].dstSet = set;
				}

				vkUpdateDescriptorSets(device.logicalDevice, uniformWriteDescriptorSet[i].size(),
					uniformWriteDescriptorSet[i].data(), 0, nullptr);

			}

		
		}

		private:

			VkCommandPool    commandPool					   = VK_NULL_HANDLE; ;
			VkClearValue     clearColor							{ 0,0,0,0 };
			VkDescriptorPool descriptorPool                    = VK_NULL_HANDLE;
			VkSemaphore      imageSemaphore, rendererSemaphore = VK_NULL_HANDLE;
			VkQueue          graphicQueue, presentQueue        = VK_NULL_HANDLE;
			VkFence			 dataTransferFence;

			uint32_t		 imageIndex						   = 0				;
			uint32_t         numberOfUniformDescriptor         = 0			    ;

			std::vector<VkCommandBuffer> commandBuffer               ;
			std::vector<VkCommandBuffer> transfererCommandBuffer,
										 indexCommandBuffer,
										 textureTransfererCmdBuff;
			std::vector<VkDescriptorSet> uniformDescriptorSets       ;

			std::vector<Buffer>			 
				realBuffers, staticTransferBuffers,
				uniformTransferBuffers,
				indexTransferBuffers,
				staticBuffers, uniformBuffers,
				indexBuffers;

			std::vector<VkDescriptorSetLayout> desSetLayouts;

			//Map is inconvinient
			std::vector<std::vector<VkWriteDescriptorSet>> uniformWriteDescriptorSet;
			std::vector<std::vector<VkDescriptorPoolSize>> uniformDescriptorPoolSize;

			std::vector<Texture>         realTextures, transfererTexture;

			Framebuffers				buffers;
			Device						device;
			GraphicPipeline				pipe;
			Swapchain				    chain;

			uint32_t currentBuffer, currentUniformBuffer = 0;
			uint32_t bufferIndex, transferIndex, texTransIndex, indexIndices = 0;

	};


}