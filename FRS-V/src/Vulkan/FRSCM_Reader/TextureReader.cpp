#include "TextureReader.h"

namespace FRS {

	Texture ReadTexture(Device device, DeviceAllocator* allocator,
		TextureType type,
		std::string path) {

		int width, height, texChannels = 0;
		int mipLevel, size = 0;
		VkFormat format;

		void* imageData = nullptr;
		char errorMess[256] = {};

		gli::texture2d tex(gli::load(path));

		width = tex.extent().x;
		height = tex.extent().y;
		mipLevel = tex.levels();
		format = (VkFormat)tex.format();
		size = tex.size();

		imageData = tex.data();

		return Texture(tex, device, imageData, width, height,
			mipLevel, 1, size,
			(VkImageType)type, format, allocator);
	}
}
