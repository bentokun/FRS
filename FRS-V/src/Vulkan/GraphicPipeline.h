#pragma once
#pragma warning (disable: 4251 4267)

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

		friend void CreateGraphicPipeline(GraphicPipeline* pipe,
			Device device, 
			DeviceAllocator allocator,
			Swapchain swapChain,
			Shader shader);

		void SetBuffers(std::vector<Buffer> &buffers) {
			this->realBuffers = buffers;
		}

		void SetLayout(VkDescriptorSetLayout layout) {
			uniformDesLayouts.push_back(layout);
		}

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
		
		VkPipeline pipeline = VK_NULL_HANDLE;
		VkPipelineLayout mLayout = VK_NULL_HANDLE;
		VkRenderPass mRenderPass = VK_NULL_HANDLE;

		std::vector<Buffer>& GetBuffers() {
			return realBuffers;
		}

		std::vector<VkDescriptorSetLayout>& GetUniformDescriptorSetLayout() {
			return uniformDesLayouts;
		}

		

		void CreateUniformDescriptorSetLayout(
			uint32_t size, Buffer buffers[],
			/*uint32_t texSize, Texture textures[],*/
			uint32_t setNumber) {

			std::vector<VkDescriptorSetLayoutBinding> bindings;
			std::vector<VkWriteDescriptorSet> writeSets;
			std::vector<VkDescriptorPoolSize> poolSizes;

			VkDescriptorBufferInfo bufferInfo = {};

			for (uint32_t i = 0; i < size; i++) {

				bindings.push_back(buffers[i].GetUniformDescriptorSetLayoutBinding());

				bufferInfo.buffer = buffers[i].buffer;
				bufferInfo.offset = buffers[i].offset;
				bufferInfo.range = sizeof(buffers[i].transferSize);

				desBufferHandle.push_back(bufferInfo);

				VkWriteDescriptorSet descriptorWrite = {};
				descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstArrayElement = 0;
				descriptorWrite.descriptorCount = bindings[i].descriptorCount;
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrite.dstBinding = buffers[i].GetUniformDescriptorSetLayoutBinding().binding;

				VkDescriptorPoolSize poolSize = {};
				poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				poolSize.descriptorCount = bindings[i].descriptorCount;

				writeSets.push_back(descriptorWrite);
				poolSizes.push_back(poolSize);
			}

			/*
			VkDescriptorImageInfo imageInfo = {};

			for (uint32_t i = 0; i < texSize; i++) {
				bindings.push_back(textures[i].GetUniformDescriptorSetLayoutBinding());

				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				bufferInfo.offset = buffers[i].offset;
				bufferInfo.range = sizeof(buffers[i].GetBindingDescriptions().stride);

				desBufferHandle.push_back(bufferInfo);

				VkWriteDescriptorSet descriptorWrite = {};
				descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstArrayElement = 0;
				descriptorWrite.descriptorCount = bindings[i].descriptorCount;
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrite.dstBinding = buffers[i].GetUniformDescriptorSetLayoutBinding().binding;

				VkDescriptorPoolSize poolSize = {};
				poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				poolSize.descriptorCount = bindings[i].descriptorCount;

				writeSets.push_back(descriptorWrite);
				poolSizes.push_back(poolSize);
			}*/

			for (uint32_t i = 0; i < writeSets.size(); i++) {
				writeSets[i].pBufferInfo = &(desBufferHandle[i]);
			}

			uniformWriteDescriptorSets.push_back(writeSets);
			uniformPoolSizes.push_back(poolSizes);

			VkDescriptorSetLayoutCreateInfo info = {};

			info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			info.bindingCount = bindings.size();
			info.pBindings = bindings.data();

			VkDescriptorSetLayout layout = {};

			VkResult result = vkCreateDescriptorSetLayout(buffers[0].getDevice().logicalDevice,
				&info, nullptr, &layout);

			uniformDesLayouts.push_back(layout);

		
		}

		uint32_t totalSet;
		std::vector<uint32_t> sizePerSet;

	private:
		
		std::vector<Buffer> realBuffers;
		std::vector<Buffer> staticBuffers;
		std::vector<Buffer> uniformBuffers;
		std::vector<Buffer> indexBuffers;

		std::vector<Texture> textures;

		std::vector<VkDescriptorSetLayout> uniformDesLayouts;

		uint32_t numberOfUniformDescriptor = 0;

		std::vector<std::vector<VkWriteDescriptorSet>> uniformWriteDescriptorSets;
		std::vector<std::vector<VkDescriptorPoolSize>> uniformPoolSizes;
		std::vector<VkDescriptorBufferInfo> desBufferHandle;

		Device device;
		DeviceAllocator allocator;

	};

	TFAPI std::vector<Buffer>
		CreateVertexBufferFromShader(Device device, DeviceAllocator allocator,
			Shader shader,
			uint32_t totalSize);

	TFAPI std::vector<Buffer>
		CreateUniformBufferFromShader(Device device, DeviceAllocator allocator,
			Shader shader,
			GraphicPipeline* pipe,
			uint32_t& totalSet,
			std::vector<uint32_t>& sizePerSet);
}