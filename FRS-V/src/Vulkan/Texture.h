#pragma once

#include <vulkan.h>
#include <gli\gli.hpp>

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

		friend void ParticallyDestroyTexture(Texture tex) {
			vkDestroyImageView(tex.device.logicalDevice, 
				tex.imageView, nullptr);
			vkDestroySampler(tex.device.logicalDevice, 
				tex.samplers[0], nullptr);
			vkDestroySampler(tex.device.logicalDevice, 
				tex.samplers[1], nullptr);
			vkDestroySampler(tex.device.logicalDevice, 
				tex.samplers[2], nullptr);
		}

		friend void FinallyDestroyTexture(Texture tex) {
			vkDestroyImage(tex.device.logicalDevice, 
				tex.image, nullptr);
			tex.allocator->deallocate(tex.block);
		}

		void CreateSampler(VkComponentMapping tComponents = 
		{VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY ,
			VK_COMPONENT_SWIZZLE_IDENTITY , VK_COMPONENT_SWIZZLE_IDENTITY }) {

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
			iCreateInfo.components = { VK_COMPONENT_SWIZZLE_R,
				VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B,
				VK_COMPONENT_SWIZZLE_A };

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

		void* GetImage() {
			return mainImage;
		}

		uint32_t GetSize() {
			return size;
		}

		Texture() {};

		Texture(gli::texture2d tex, Device device, void* image, int texWidth, int texHeight,
			int mipmapLevel, int layers, int size,
			VkImageType type, VkFormat format, DeviceAllocator* alloc);
			
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

		std::vector<VkSampler> GetSamplers() {
			return samplers;
		}

		int GetMipLevel() {
			return mipLevel;
		}

		gli::texture2d tex;
		VkImage image = VK_NULL_HANDLE;

	private:

		VkFormat format;

		int texWidth, texHeight, size = 0;
		void* mainImage = nullptr;

		VkSubresourceLayout           subLayout;
		VkImageSubresource            subresource;

		VkImageView imageView = VK_NULL_HANDLE;
		std::vector<VkSampler> samplers = { VK_NULL_HANDLE };

		uint32_t mipLevel, layers;

		Block block;
		DeviceAllocator* allocator;
		Device device;

	};

};