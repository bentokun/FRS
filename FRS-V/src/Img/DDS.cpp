#include "DDS.h"

namespace FRS {

	std::vector<unsigned char> LoadDDS(const char* inpName,
		int* mipMapLevel,
		std::vector<int>* width,
		std::vector<int>* height,
		std::vector<int>* size,
		VkFormat* vkFormat,
		char errorMessage[256])
	{
		int magic;
		std::vector<unsigned char> returnData;
		unsigned char* temp;
		unsigned char** data;
		int bytes[16];
		

		std::ifstream f(inpName, std::ios::binary);

		f.read(reinterpret_cast<char*>(&magic), sizeof(magic));

		if (magic != MAGIC_DDS) {
			return std::vector<unsigned char>();
		}
		else {
			DDSurfaceDes2 header = {};

			f.read(reinterpret_cast<char*>(&header), sizeof(header));

			*mipMapLevel = header.mipmapCount;

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

					return std::vector<unsigned char>();
				}
			}
			else if (header.pixelFormat.rgbBitCount == 32) {
				format = DDS_FORMAT_RGBA8;

				imageBytes = imageBytes*header.pixelFormat.rgbBitCount / 8;
			}
			else
			{
				errorMessage = "Can't specify DDS format";
				return std::vector<unsigned char>();
			}

			int w = header.width; int h = header.height;
			int mipmapLvl = *mipMapLevel;

			data = reinterpret_cast<unsigned char**>(malloc(mipmapLvl * sizeof(unsigned char*)));

			for (int i = 0; i < header.mipmapCount && i < 16; i++) {
				if (w == 0) w = 1;
				if (h == 0) h = 1;

				if (format == DDS_FORMAT_RGBA8) {
					imageBytes = w*h * 4;
					size->push_back(imageBytes);
				}
				else {
					imageBytes = ((w + 3) / 4)*((h + 3) / 4)*blockSize;
					size->push_back(imageBytes);
				}

				if (i == 0) {
					temp = (unsigned char*)malloc(imageBytes);
				}

				bytes[i] = imageBytes;
				data[i] = (unsigned char*)malloc(bytes[i]);

				if (!data[i]) {
					errorMessage = "Out of memory!";
					return std::vector<unsigned char>();
				}

				if (format != DDS_FORMAT_RGBA8) {
					f.read(reinterpret_cast<char*>(data[i]), imageBytes);
					
					/*int j, widthBytes, k;
					unsigned char *s, *d;
					widthBytes = ((w + 3) / 4)*blockSize;
					s = temp;
					d = data[i] + ((h + 3) / 4 - 1)*widthBytes;

					for (j = 0; j < ((h + 3) / 4); j++) {
						memcpy(d, s, widthBytes);
					}

					s += widthBytes;
					d -= widthBytes;
					*/
				}
				width->push_back(w);
				height->push_back(h);

				w /= 2; h /= 2;
			}

		}

		for (int i = 0; i < *mipMapLevel; i++) {
			for (int j = 0; j < bytes[i]; j++) {
				returnData.push_back(data[i][j]);
			}
			free(data[i]);
		}

		free(temp);
		free(data);
		f.close();
		return returnData;
	}

	void ConvertARGBtoRGBA(unsigned char* block, int size) {

		unsigned char temp;
		size /= 4;

		for (int i = 0; i < size; i++) {
			temp = block[2];
			block[2] = block[0];
			block[0] = temp;
			block += 4;
		}
	}

}