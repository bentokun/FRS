#include "Texture.h"

namespace FRS {

	Texture::Texture(Device device, std::vector<unsigned char> image, 
		std::vector<int> texWidth, std::vector<int> texHeight,
		std::vector<int> size,
		int mipmapLevel, int layers,
		VkImageType type, VkFormat format, DeviceAllocator* alloc,
		VkImageUsageFlags usageFlag) :
		texWidth(texWidth),
		texHeight(texHeight),
		mainImage(image),
		device(device),
		allocator(alloc),
		mipLevel(mipmapLevel),
		layers(layers),
		size(size),
		format(format)
	{
			for (uint32_t i = 0; i < mipLevel; i++) {
				totalSize += size[i];
			}
			
			VkImageCreateInfo imageCreateInfo = {};
			imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
			imageCreateInfo.format = format;
			imageCreateInfo.mipLevels = mipmapLevel;
			imageCreateInfo.arrayLayers = 1;
			imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageCreateInfo.extent.width = texWidth[0];
			imageCreateInfo.extent.height = texHeight[0];
			imageCreateInfo.extent.depth = 1;
			imageCreateInfo.usage = usageFlag;

			VkResult res1 = vkCreateImage(device.logicalDevice,
				&imageCreateInfo, nullptr, &this->image);

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
			
			this->block = allocator->allocate(mMemReq.size, mMemReq.alignment, memType);
			vkBindImageMemory(device.logicalDevice, this->image, block.memory, block.offSet);

	}
	

	void Destroy(Texture texture) {
		vkDestroyImage(texture.device.logicalDevice,
			texture.image, nullptr);
		vkDestroyImageView(texture.device.logicalDevice, texture.imageView, nullptr);
		
		for (uint32_t i = 0; i < texture.samplers.size(); i++) {
			vkDestroySampler(texture.device.logicalDevice,
				texture.samplers[i], nullptr);
		}

		vkFreeMemory(texture.device.logicalDevice, texture.block.memory, nullptr);

	}

	bool operator == (Texture texture0, Texture texture) {

		if (texture0.imageView == texture.imageView
			&& texture0.block == texture.block)

			return true;

		return false;
	}

	bool Texture::operator == (Texture* texture0) {

		if (texture0->imageView == this->imageView
			&& texture0->block == this->block)

			return true;

		return false;
	}

	void DestroyTextureViews(Texture* tex) {
		vkDestroyImageView(tex->device.logicalDevice,
			tex->imageView, nullptr);
	}

	void DestroyTextureSampler(Texture* tex) {
		vkDestroySampler(tex->device.logicalDevice,
			tex->samplers[0], nullptr);
		vkDestroySampler(tex->device.logicalDevice,
			tex->samplers[1], nullptr);
		vkDestroySampler(tex->device.logicalDevice,
			tex->samplers[2], nullptr);
	}

	void FinallyDestroyTexture(Texture* tex) {
		vkDestroyImage(tex->device.logicalDevice,
			tex->image, nullptr);
		tex->allocator->deallocate(tex->block);
	}

	void Texture::CreateImageView(VkComponentMapping tComponents,
		VkImageAspectFlags flag) {

		VkImageViewCreateInfo iCreateInfo = {};
		iCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		iCreateInfo.image = this->image;
		iCreateInfo.subresourceRange.aspectMask = flag;
		iCreateInfo.subresourceRange.baseMipLevel = 0;
		iCreateInfo.subresourceRange.levelCount = mipLevel;
		iCreateInfo.subresourceRange.baseArrayLayer = 0;
		iCreateInfo.subresourceRange.layerCount = 1;
		iCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		iCreateInfo.format = format;
		iCreateInfo.components = { VK_COMPONENT_SWIZZLE_R,
			VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B,
			VK_COMPONENT_SWIZZLE_A };

		vkCreateImageView(device.logicalDevice, &iCreateInfo, nullptr,
			&imageView);
	}

	void Texture::CreateSampler() {


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
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.minLod = 0;
		samplerInfo.maxLod = 0;
		samplerInfo.mipLodBias = 0;

		VkResult res = vkCreateSampler(device.logicalDevice, &samplerInfo, nullptr,
			&samplers[0]);

		FRS_S_ASSERT(res != VK_SUCCESS);

		samplerInfo.maxLod = mipLevel;

		VkResult res2 = vkCreateSampler(device.logicalDevice, &samplerInfo, nullptr,
			&samplers[1]);


		FRS_S_ASSERT(res2 != VK_SUCCESS);

		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 16;

		vkCreateSampler(device.logicalDevice, &samplerInfo, nullptr,
			&samplers[2]);


	}
}