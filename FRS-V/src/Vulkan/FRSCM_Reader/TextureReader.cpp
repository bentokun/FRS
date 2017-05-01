#include "TextureReader.h"

namespace FRS {

	Texture ReadTexture(Device device, DeviceAllocator allocator,
		TextureType type,
		std::string path) {

		int width, height, texChannels;
		int mipLevel;
		VkFormat format;

		unsigned char** imageData = nullptr;
		char errorMess[256] = {};

		if (path.substr(path.find_last_of('.') + 1) == "dds") {
			imageData = LoadDDS(path.c_str(),
				&width, &height, &mipLevel, &format,
				errorMess);
		}

		return Texture(device, image, width, height,
			(VkImageType)type, true, allocator);
	}
}
