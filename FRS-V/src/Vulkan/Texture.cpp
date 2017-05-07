#include "Texture.h"

namespace FRS {

	Texture::Texture(gli::texture2d tex, Device device, void* image, int texWidth, int texHeight,
		int mipmapLevel, int layers,
		int size,
		VkImageType type, VkFormat format, DeviceAllocator* alloc) :
		texWidth(texWidth),
		texHeight(texHeight),
		mainImage(image),
		device(device),
		allocator(alloc),
		mipLevel(mipmapLevel),
		layers(layers),
		size(size),
		tex(tex),
		format(format)
	{
			VkDeviceSize imageSize = texWidth* texHeight * 4;

			VkImageCreateInfo imageCreateInfo = {};
			imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
			imageCreateInfo.format = format;
			imageCreateInfo.mipLevels = tex.levels();
			imageCreateInfo.arrayLayers = 1;
			imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			// Set initial layout of the image to undefined
			imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageCreateInfo.extent.width = this->texWidth;
			imageCreateInfo.extent.height = texHeight;
			imageCreateInfo.extent.depth = 1;
			imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

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
}