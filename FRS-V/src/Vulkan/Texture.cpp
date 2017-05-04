#include "Texture.h"

namespace FRS {

	Texture::Texture(Device device, unsigned char** image, int texWidth, int texHeight,
		int mipmapLevel, int layers,
		int size,
		VkImageType type, VkFormat format, DeviceAllocator alloc) :
		mainImage(image),
		device(device),
		allocator(alloc),
		mipLevel(mipmapLevel),
		layers(layers),
		size(size)
	{
			VkDeviceSize imageSize = texWidth* texHeight * 4;

			VkImageCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			createInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
			createInfo.extent.width = texWidth;
			createInfo.extent.height = texHeight;
			createInfo.arrayLayers = layers;
			createInfo.format = format;
			createInfo.mipLevels = mipLevel;
			createInfo.imageType = type;
			createInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
			createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.samples = VK_SAMPLE_COUNT_1_BIT;

			VkResult res1 = vkCreateImage(device.logicalDevice,
				&createInfo, nullptr, &this->image);

			FRS_S_ASSERT(res1 != VK_SUCCESS);

			VkMemoryRequirements mMemReq{};
			VkPhysicalDeviceMemoryProperties  mMemProp{};

			vkGetImageMemoryRequirements(device.logicalDevice,
				this->image, &mMemReq);

			vkGetPhysicalDeviceMemoryProperties(device.physicalDevice,
				&mMemProp);

			auto lamdaFunc = [&](VkMemoryPropertyFlags flag) -> int {
				for (uint32_t i = 0; i < mMemProp.memoryTypeCount; ++i)
					if (mMemReq.memoryTypeBits & (1 << i) &&
						((mMemProp.memoryTypes[i].propertyFlags & flag) == flag)) {
						return i;
					}
				return -1;
			};

			int memType;

			memType = lamdaFunc(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			
			this->block = allocator.allocate(mMemReq.size, mMemReq.alignment, memType);
			vkBindImageMemory(device.logicalDevice, this->image, block.memory, block.offSet);

			
			/*
			if (mipmapLevel == 1) {
				VkImageSubresource subresource = {};
				subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				subresource.mipLevel = 0;
				subresource.arrayLayer = 0;

				VkSubresourceLayout layout = {};
				vkGetImageSubresourceLayout(device.logicalDevice,
					this->image, &subresource,
					&layout);

				vkMapMemory(device.logicalDevice, block.memory,
					0, imageSize, 0, &dataPointer);

				if (layout.depthPitch == texWidth * 4) {
					uint8_t* dataBytes = reinterpret_cast<uint8_t*>(dataPointer);

					for (int y = 0; y < texHeight; y++) {
						memcpy(&dataBytes[y * layout.rowPitch],
							&mainImage[0][y * texWidth * 4],
							texWidth * 4);
					}
				}
				else
					memcpy(dataPointer, mainImage[0], imageSize);

				vkUnmapMemory(device.logicalDevice, block.memory);
			}
			else
			{
				Buffer buffer;
				CreateBuffer(buffer, device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
					sizeof(mainImage), false, allocator);

				vkMapMemory(device.logicalDevice, buffer.GetBlock().memory,
					0, sizeof(mainImage), 0, &buffer.dataPointer);
				memcpy(dataPointer, &mainImage, sizeof(mainImage));
				vkUnmapMemory(device.logicalDevice, buffer.GetBlock().memory);
				
				VkBufferImageCopy bufferCopyRegion = {};
				bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				bufferCopyRegion.imageSubresource.mipLevel = mipLevel;
				bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
				bufferCopyRegion.imageSubresource.layerCount = 1;
				bufferCopyRegion.imageExtent.width = texWidth;
				bufferCopyRegion.imageExtent.height = texHeight;
				bufferCopyRegion.imageExtent.depth = 1;

			}
			
			*/
			
			VkImageViewCreateInfo iCreateInfo = {};
			iCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			iCreateInfo.image = this->image;
			iCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			iCreateInfo.subresourceRange.baseMipLevel = 0;
			iCreateInfo.subresourceRange.levelCount = mipLevel;
			iCreateInfo.subresourceRange.baseArrayLayer = 0;
			iCreateInfo.subresourceRange.layerCount = 1;
			iCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			iCreateInfo.format = format;
				
			vkCreateImageView(device.logicalDevice, &iCreateInfo, nullptr,
				&imageView);

			samplers.resize(3);

			VkSamplerCreateInfo samplerInfo = {};
			samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.minFilter = VK_FILTER_LINEAR;
			samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.anisotropyEnable = VK_FALSE;
			samplerInfo.maxAnisotropy = 1.0f;
			samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
			samplerInfo.unnormalizedCoordinates = VK_FALSE;
			samplerInfo.compareEnable = VK_FALSE;
			samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerInfo.minLod = 0;
			samplerInfo.maxLod = 0;
			samplerInfo.mipLodBias = 0;
			
			vkCreateSampler(device.logicalDevice, &samplerInfo, nullptr,
				&samplers[0]);

			samplerInfo.maxLod = mipLevel;

			vkCreateSampler(device.logicalDevice, &samplerInfo, nullptr,
				&samplers[1]);

			samplerInfo.anisotropyEnable = VK_TRUE;
			samplerInfo.maxAnisotropy = 16;

			vkCreateSampler(device.logicalDevice, &samplerInfo, nullptr,
				&samplers[3]);

			
	}
	

	void Destroy(Texture texture) {
		vkDestroyImageView(texture.device.logicalDevice, texture.imageView, nullptr);
		vkFreeMemory(texture.device.logicalDevice, texture.block.memory, nullptr);

	}

	bool operator == (Texture texture0, Texture texture) {

		if (texture0.imageView == texture.imageView
			&& texture0.block == texture.block)

			return true;

		return false;
	}
}