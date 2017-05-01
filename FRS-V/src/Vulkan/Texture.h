#pragma once

#include <vulkan.h>

#include "Buffer.h"

#if _WIN32
#ifdef FRSV_EXPORTS
#define TFSAPI __declspec(dllexport)
#else
#define TFSAPI __declspec(dllimport)
#endif

#else
#define TFSAPI
#endif

enum TextureType {
	TEXTURE_1D = 0, 
	TEXTURE_2D = 1 ,
	TEXTURE_3D = 2
};

namespace FRS {

	class TFSAPI Texture {

	public:

		void* dataPointer;

		Block& GetBlock() {
			return block;
		}

		VkSubresourceLayout& GetSubresourceLayout() {
			return subLayout;
		}

		VkImageSubresource& GetImageSubresource() {
			return subresource;
		}


		uint32_t GetWidth() {
			return texWidth;
		}

		uint32_t GetHeight() {
			return texHeight;
		}

		unsigned char** GetImage() {
			return mainImage;
		}

		Texture() {};

		Texture(Device device, unsigned char** image, int texWidth, int texHeight,
			int mipmapLevel, int layers,
			VkImageType type, VkFormat format,
			bool local, DeviceAllocator alloc);
			
		friend void Destroy(Texture texture);
		friend bool operator == (Texture texture0, Texture texture);

		VkDescriptorSetLayoutBinding& GetUniformDescriptorSetLayoutBinding() {
			return layoutBinding;
		}

		VkImage& GetVkImage() {
			return image;
		}
		
	private:


		int texWidth, texHeight;
		unsigned char** mainImage;

		VkDescriptorSetLayoutBinding  layoutBinding;
		VkSubresourceLayout           subLayout;
		VkImageSubresource            subresource;

		VkImage image, realImage = VK_NULL_HANDLE;
		VkImageView imageView = VK_NULL_HANDLE;
		VkSampler sampler = VK_NULL_HANDLE;

		uint32_t mipLevel, layers;

		Block block;
		DeviceAllocator allocator;
		Device device;

	};

};