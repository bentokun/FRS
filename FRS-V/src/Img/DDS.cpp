#include "DDS.h"

namespace FRS {

	unsigned char **LoadDDS(const char* inpName,
		int* width,
		int* height, int* mimMapLevel,
		VkFormat* vkFormat, int* size, char errorMessage[256])
	{
		int magic;
		unsigned char* temp = nullptr;
		unsigned char** data;
		int bytes[16];

		std::ifstream f(inpName, std::ios::binary);

		f.read(reinterpret_cast<char*>(&magic), sizeof(magic));

		if (magic != MAGIC_DDS) {
			return nullptr;
		}
		else {
			DDSurfaceDes2 header = {};
			
			f.read(reinterpret_cast<char*>(&header), sizeof(header));

			*width = header.width;
			*height = header.height;
			*mimMapLevel = header.mipmapCount;
			
			DDSFormatType format = {};

			int imageBytes = header.width * header.height;
			int blockSize = 16;

			if (header.pixelFormat.flags & DDSPF_FOURCC) {
				unsigned int fourCC;  /*Get data format. It's 4 bytes word*/
				fourCC = header.pixelFormat.fourCC;

				format = DDS_FORMAT_YCOCG;

				if (fourCC == ID_DXT1) {
					blockSize = 8;
					*vkFormat = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
				}
				else if (fourCC == ID_DXT3) {
					*vkFormat = VK_FORMAT_BC2_UNORM_BLOCK;
				}
				else if (fourCC == ID_DXT5) {
					*vkFormat = VK_FORMAT_BC3_UNORM_BLOCK;
				}
				else {
					char buf[5];
					buf[0] = fourCC & 255;
					buf[1] = (fourCC >> 8) & 255;
					buf[2] = (fourCC >> 16) & 255;
					buf[3] = (fourCC >> 24) & 255;
					buf[4] = 0;

					errorMessage = buf;

					return nullptr;
				}
			}
			else if (header.pixelFormat.rgbBitCount == 32) {
				format = DDS_FORMAT_RGBA8;

				imageBytes = imageBytes*header.pixelFormat.rgbBitCount / 8;
			}
			else
			{
				errorMessage = "Can't specify DDS format";
				return nullptr;
			}

			int w = *width; int h = *height;
			int mipmapLvl = *mimMapLevel;

			data = reinterpret_cast<unsigned char**>(malloc(mipmapLvl * sizeof(unsigned char*)));

			for (uint32_t i = 0; i < header.mipmapCount && i < 16; i++) {
				if (w == 0) w = 1;
				if (h == 0) h = 1;

				if (format == DDS_FORMAT_RGBA8) {
					imageBytes = w*h * 4;
					*size += imageBytes;
				}
				else {
					imageBytes = ((w + 3) / 4)*((h + 3) / 4)*blockSize;
					*size += imageBytes;
				}

				if (i == 0) {
					temp = (unsigned char*)malloc(imageBytes);
				}

				bytes[i] = imageBytes;
				data[i] = (unsigned char*)malloc(bytes[i]);

				if (!data[i]) {
					errorMessage = "Out of memory!";
					return nullptr;
				}

				if (format != DDS_FORMAT_RGBA8) {
					f.read(reinterpret_cast<char*>(temp), imageBytes);
					int j, widthBytes, k;
					unsigned char *s, *d;
					widthBytes = ((w + 3) / 4)*blockSize;
					s = temp;
					d = data[i] + ((h + 3) / 4 - 1)*widthBytes;

					for (j = 0; j < ((h + 3) / 4 - 1)*widthBytes; j++) {
						memcpy(d, s, widthBytes);
					}

					s += widthBytes;
					d -= widthBytes;

				}

				w /= 2; h /= 2;
			}

		}


		free(temp);
		f.close();
		return data;
	}
	
	void UnloadDDS(unsigned char** block, uint32_t numberMips) {
		for (uint32_t i = 0; i < numberMips; i++) {
			free(block[i]);
		}

		free(block);
	}

}