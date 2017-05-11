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

		void* dataPointer = nullptr;

		friend void DestroyTextureViews(Texture* tex);
		friend void DestroyTextureSampler(Texture* tex);
		friend void FinallyDestroyTexture(Texture* tex);

		void CreateImageView(VkComponentMapping tComponents =
		{ VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY ,
			VK_COMPONENT_SWIZZLE_IDENTITY , VK_COMPONENT_SWIZZLE_IDENTITY },
			VkImageAspectFlags flag = VK_IMAGE_ASPECT_COLOR_BIT);

		void CreateSampler();

		Block& GetBlock() {
			return block;
		}

		VkSubresourceLayout& GetSubresourceLayout() {
			return subLayout;
		}

		VkImageSubresource& GetImageSubresource() {
			return subresource;
		}

		std::vector<int> GetWidth() {
			return texWidth;
		}

		std::vector<int> GetHeight() {
			return texHeight;
		}

		unsigned char* GetImage() {
			return mainImage.data();
		}

		std::vector<int> GetSize() {
			return size;
		}

		Texture() {};

		Texture(Device device, std::vector<unsigned char> image, std::vector<int> texWidth, std::vector<int> texHeight,
			std::vector<int> size,
			int mipmapLevel, int layers,
			VkImageType type, VkFormat format, DeviceAllocator* alloc,
			VkImageUsageFlags usageFlag = VK_IMAGE_USAGE_TRANSFER_DST_BIT|VK_IMAGE_USAGE_SAMPLED_BIT);
			
		friend void Destroy(Texture texture);
		friend bool operator == (Texture texture0, Texture texture);
	
		bool operator == (Texture* texture0);

		VkDescriptorSetLayoutBinding setUniformLayoutBinding{};
		VkDescriptorPoolSize poolSize;
		
		VkFormat& GetFormat(){
			return format;
		}

		VkImage &GetVkImage() {
			return image;
		}

		VkImageView &GetImageViews() {
			return imageView;
		}

		std::vector<VkSampler> GetSamplers() {
			return samplers;
		}

		int GetMipLevel() {
			return mipLevel;
		}

		uint32_t GetTotalSize() {
			return totalSize;
		}

	private:

		VkFormat format;
		VkImage image = VK_NULL_HANDLE;

		std::vector<int> texWidth, texHeight, size;
		std::vector<unsigned char> mainImage;

		VkSubresourceLayout           subLayout;
		VkImageSubresource            subresource;

		VkImageView imageView = VK_NULL_HANDLE;
		std::vector<VkSampler> samplers = { VK_NULL_HANDLE };

		uint32_t mipLevel, layers, totalSize = 0;

		Block block;
		DeviceAllocator* allocator;
		Device device;

	};

};