#pragma once
#pragma warning (disable: 4251 4267)

#include <array>
#include <map>

#include "Rules.h"
#include "Shader.h"
#include "Device.h"
#include "SwapChain.h"
#include "Buffer.h"
#include "Texture.h"


#ifdef _WIN32
#ifdef FRSV_EXPORTS
#define TFSAPI __declspec(dllexport)
#else
#define TFSAPI __declspec(dllimport)
#endif
#endif

namespace FRS {

	class TFSAPI GraphicPipeline {
	public:

		GraphicPipeline() {};

		friend void
			CreateUniformBufferFromShader(Device device,
				DeviceAllocator* allocator,
				Shader* shader,
				GraphicPipeline* pipe,
				uint32_t& totalSet,
				std::vector<uint32_t>& sizePerSet,
				std::vector<uint32_t>& sizeTexPerSet);

		friend void CreateGraphicPipeline(GraphicPipeline* pipe,
			DeviceAllocator* allocator,
			Shader* shader,
			Device device, 
			Swapchain swapChain);
#pragma region GETTER

		std::vector<std::vector<VkWriteDescriptorSet>> GetWriteDescriptorLayout() {
			return uniformWriteDescriptorSets;
		}

		std::vector<std::vector<VkDescriptorPoolSize>> GetDescriptorPoolSize() {
			return uniformPoolSizes;
		}

		std::vector<Buffer> &GetStaticBuffer() {
			return staticBuffers;
		}

		std::vector<Buffer> &GetUniformBuffer() {
			return uniformBuffers;
		}

		std::vector<Buffer> &GetIndexBuffer() {
			return indexBuffers;
		}

		uint32_t NumberOfUniformDescriptor() {
			return numberOfUniformDescriptor;
		}

		friend void DestroyGraphicPipeline(
			GraphicPipeline* pipe);

		std::vector<Buffer>& GetBuffers() {
			return realBuffers;
		}

		std::vector<VkDescriptorSetLayout>& GetUniformDescriptorSetLayout() {
			return uniformDesLayouts;
		}

		std::vector<Texture*>& GetTextures() {
			return textures;
		}

		VkPipeline& GetPipeline() {
			return pipeline;
		}

		VkRenderPass& GetRenderPass() {
			return mRenderPass;
		}

		VkPipelineLayout& GetPipelineLayout() {
			return mLayout;
		}
#pragma endregion
		void GenerateWriteDescriptor(std::vector<Buffer> buffers,
			std::vector<Texture*> texs, uint32_t set);

		void CreateUniformDescriptorSetLayout(
			std::vector<Buffer> buffers,
			std::vector<Texture*> texs,
			uint32_t setNumber);

		uint32_t totalSet;
		std::vector<uint32_t> sizePerSet;
		std::vector<uint32_t> sizeTexPerSet;

		std::vector<std::vector<VkDescriptorBufferInfo>> desBufferHandle;
		std::vector<std::vector<VkDescriptorImageInfo>> desImageHandle;

	private:

		VkPipeline pipeline = VK_NULL_HANDLE;
		VkPipelineLayout mLayout = VK_NULL_HANDLE;
		VkRenderPass mRenderPass = VK_NULL_HANDLE;

		std::vector<Buffer> realBuffers;
		std::vector<Buffer> staticBuffers;
		std::vector<Buffer> uniformBuffers;
		std::vector<Buffer> indexBuffers;

		std::vector<Texture*> textures;

		std::vector<VkDescriptorSetLayout> uniformDesLayouts;

		uint32_t numberOfUniformDescriptor = 0;

		std::vector<std::vector<VkWriteDescriptorSet>> uniformWriteDescriptorSets;
		std::vector<std::vector<VkDescriptorPoolSize>> uniformPoolSizes;
		

		Device device;
		DeviceAllocator* allocator;

	};

	TFSAPI std::vector<Buffer>
		CreateVertexBufferFromShader(Device device,
			DeviceAllocator* allocator,
			Shader* shader,
			uint32_t totalSize);


}