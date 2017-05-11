#include "TextureReader.h"

namespace FRS {

	Texture ReadTexture(Device device, DeviceAllocator* allocator,
		TextureType type,
		std::string path) {

		std::vector<int> width, height, size;
		int mipLevel, texChannels = 0;
		VkFormat format;

		char errorMess[256] = {};

		std::vector<unsigned char> image = LoadDDS(path.c_str(),
			&mipLevel, &width, &height, &size, &format, errorMess);
		

		return Texture(device, image, width, height, size,
			mipLevel, 1, (VkImageType)type, format, allocator);
	}
}
