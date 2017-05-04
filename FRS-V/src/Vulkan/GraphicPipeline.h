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

		friend void
			CreateUniformBufferFromShader(Device device, DeviceAllocator allocator,
				Shader shader,
				GraphicPipeline* pipe,
				uint32_t& totalSet,
				std::vector<uint32_t>& sizePerSet,
				std::vector<uint32_t>& sizeTexPerSet);

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

		std::vector<Texture>& GetTextures() {
			return textures;
		}

		void CreateUniformDescriptorSetLayout(
			std::vector<Buffer> buffers,
			/*uint32_t texSize, Texture textures[],*/
			std::vector<Texture> texs,
			uint32_t setNumber) {

			std::vector<VkDescriptorSetLayoutBinding> bindings;
			std::vector<VkWriteDescriptorSet> writeSets;
			std::vector<VkDescriptorPoolSize> uniformPoolSize;

			for (uint32_t i = 0; i < buffers.size(); i++) {
				bindings.push_back(buffers[i].setLayoutBinding);

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
				bindings.push_back(texs[i].setUniformLayoutBinding);

				VkDescriptorImageInfo imageInfo = {};
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfo.imageView = texs[i].GetImageViews();

				std::vector<VkDescriptorImageInfo> imageInfos;
				for (uint32_t j = 0; j < texs[i].GetSamplers().size(); j++) {
					imageInfo.sampler = texs[i].GetSamplers()[i];

					imageInfos.push_back(imageInfo);
				}

				desImageHandle.push_back(imageInfos);

				VkWriteDescriptorSet descriptorSet{};
				descriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorSet.descriptorCount = texs[i].GetSamplers().size();
				descriptorSet.pImageInfo = desImageHandle[desImageHandle.size() - 1].data();
				descriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorSet.dstBinding = texs[i].setUniformLayoutBinding.binding;
				descriptorSet.dstArrayElement = 0;

				writeSets.push_back(descriptorSet);

				uniformPoolSize.push_back(texs[i].poolSize);
			}
		
			uniformWriteDescriptorSets.push_back(writeSets);
			uniformPoolSizes.push_back(uniformPoolSize);

			VkDescriptorSetLayoutCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			createInfo.bindingCount = bindings.size();
			createInfo.pBindings = bindings.data();
			
			VkDescriptorSetLayout uniformDesLayout;

			vkCreateDescriptorSetLayout(device.logicalDevice,
				&createInfo, nullptr, &uniformDesLayout);

			uniformDesLayouts.push_back(uniformDesLayout);

		}

		uint32_t totalSet;
		std::vector<uint32_t> sizePerSet;
		std::vector<uint32_t> sizeTexPerSet;

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
		std::vector<std::vector<VkDescriptorBufferInfo>> desBufferHandle;
		std::vector<std::vector<VkDescriptorImageInfo>> desImageHandle;

		Device device;
		DeviceAllocator allocator;

	};

	TFAPI std::vector<Buffer>
		CreateVertexBufferFromShader(Device device, DeviceAllocator allocator,
			Shader shader,
			uint32_t totalSize);


}