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

		uint32_t GetSize() {
			return size;
		}

		Texture() {};

		Texture(Device device, unsigned char** image, int texWidth, int texHeight,
			int mipmapLevel, int layers, int size,
			VkImageType type, VkFormat format, DeviceAllocator alloc);
			
		friend void Destroy(Texture texture);
		friend bool operator == (Texture texture0, Texture texture);

		VkDescriptorSetLayoutBinding setUniformLayoutBinding{};
		VkDescriptorPoolSize poolSize;
		
		VkImage &GetVkImage() {
			return image;
		}

		VkImageView &GetImageViews() {
			return imageView;
		}

		std::vector<VkSampler> &GetSamplers() {
			return samplers;
		}

		int GetMipLevel() {
			return mipLevel;
		}

	private:


		int texWidth, texHeight, size;
		unsigned char** mainImage;

		VkSubresourceLayout           subLayout;
		VkImageSubresource            subresource;

		VkImage image, realImage = VK_NULL_HANDLE;
		VkImageView imageView = VK_NULL_HANDLE;
		std::vector<VkSampler> samplers = { VK_NULL_HANDLE };

		uint32_t mipLevel, layers;

		Block block;
		DeviceAllocator allocator;
		Device device;

	};

};