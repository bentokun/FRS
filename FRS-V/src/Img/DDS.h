#pragma once

#include <fstream>
#include <ostream>

#include <Vulkan.h>
#include <string.h>

#ifdef _WIN32
	#ifdef FRSV_EXPORTS
		#define TFSAPI __declspec(dllexport)
	#else
		#define TFSAPI __declspec(dllimport)
	#endif
	#else

	#define TFSAPI

#endif

#define FOURCC(a,b,c,d)\
((unsigned int)(unsigned int)(a)) | \
((unsigned int)(unsigned int)(b)<<8) | \
((unsigned int)(unsigned int)(c)<<16) | \
((unsigned int)(unsigned int)(d)<<24) | 

#pragma region DDSINFORM
#define DDS_HEADERSIZE   128
#define DDS_CAPABILITES  0x00000001
#define DDS_HEIGHT       0x00000002
#define DDS_WIDTH        0x00000004
#define DDS_PITCH        0x00000008
#define DDS_PIXEL_FORMAT 0x00001000
#define DDS_MIPMAPCOUNT  0x00020000
#define DDS_LINEARSIZE   0x00080000
#define DDS_DEPTH        0x00800000

#define DDSPF_ALPHAPIXELS  0x00000001
#define DDSPF_ALPHA        0x00000002
#define DDSPF_FOURCC       0x00000004
#define DDSPF_PALETTEINDEX 0x00000020
#define DDSPF_RGB          0x00000040
#define DDSPF_LUMINACE     0x00020000

#define DDSCAPS_COMPLEX    0x00000008
#define DDSCAPS_TEXTURE    0x00001000
#define DDSCAPS_MIPMAP     0x00400000

#define DDSCAPS2_CUPEMAP            0x00000200
#define DDSCAPS2_CUPEMAP_POSITIVEX  0x00000400
#define DDSCAPS2_CUPEMAP_NEGATIVEX  0x00000800
#define DDSCAPS2_CUPEMAP_POSITIVEY  0x00001000
#define DDSCAPS2_CUPEMAP_NEGATIVEY  0x00002000
#define DDSCAPS2_CUPEMAP_POSITIVEZ  0x00004000
#define DDSCAPS2_CUPEMAP_NEGATIVEZ  0x00008000
#define DDSCAPS2_VOLUME             0x00200000

#define MAGIC_DDS				    0x20534444    

#define ID_DXT1						0x31545844
#define ID_DXT3						0x33545844
#define ID_DXT5						0x35545844
#pragma endregion

enum DDSCompressionType {
	DDS_COMPRESS_NONE = 0,
	DDS_COMPRESS_BC1,
	DDS_COMPRESS_BC2,
	DDS_COMPRESS_BC3,
	DDS_COMPRESS_BC3N,
	DDS_COMPRESS_BC4,
	DDS_COMPRESS_BC5,
	DDS_COMPRESS_AEXP,
	DDS_COMPRESS_YCOG,
	DDS_COMPRESS_YCOCGS,
	DDS_COMPRESS_MAX
};

enum DDSSaveType {
	DDS_SAVE_SELECTED_LAYER = 0,
	DDS_SAVE_CUPEMAP,
	DDS_SAVE_VOLUMEMAP,
	DDS_SAVE_MAX
};

enum DDSFormatType {
	DDS_FORMAT_DEFAULT = 0,
	DDS_FORMAT_RGB8,
	DDS_FORMAT_RGBA8,
	DDS_FORMAT_BGR8,
	DDS_FORMAT_ABGR8,
	DDS_FORMAT_R5G6B5,
	DDS_FORMAT_RGBA4,
	DDS_FORMAT_RGB5A1,
	DDS_FORMAT_RGB10A2,
	DDS_FORMAT_R3G3B2,
	DDS_FORMAT_A8,
	DDS_FORMAT_L8,
	DDS_FORMAT_L8A8,
	DDS_FORMAT_AEXP,
	DDS_FORMAT_YCOCG,
	DDS_FORMAT_MAX,
};

enum DDSColorType {
	DDS_COLOR_DEFAULT = 0,
	DDS_COLOR_DISTANCE,
	DDS_COLOR_LUMINACE,
	DDS_COLOR_INSERT_BBOX,
	DDS_COLOR_MAX
};

enum DDSMipmapType {
	DDS_MIPMAP_DEFAULT = 0,
	DDS_MIPMAP_NEAREST,
	DDS_MIPMAP_BOX,
	DDS_MIPMAP_BILINEAR,
	DDS_MIPMAP_BICUBIC,
	DDS_MIPMAP_LANCZOS,
	DDS_MIPMAP_MAX
};

namespace FRS {

	struct DDPixelFormat {
		int size;
		int flags;
		int fourCC;
		int rgbBitCount;
		int rBitMask, gBitMask, bBitMask;
		int rgbaBitMask;
	};

	struct DDCaps2 {
		int caps1;
		int caps2;
		int reserved[2];
	};

	struct DDSurfaceDes2 {
		int size;
		int flags;
		int height;
		int width;
		int pitchOrLinear;
		int depth;
		int mipmapCount;
		
		int reserved1[11];

		DDPixelFormat pixelFormat;
		DDCaps2 capabilites;


		int reserved2;
	};

	//@L-------------------------------------------------L@
	//@ Folder: Img
	//@ File Loader: DDS (Vulkan + DirectX)
	//@ Return image, include [mipmapLevel] of mipmap
	TFSAPI unsigned char **LoadDDS(const char* inpName,
		int* width,
		int* height, int* mimMapLevel,
		VkFormat* vkFormat, 
		int* size,
		char errorMessage[256]);

	TFSAPI void UnloadDDS(unsigned char** block, uint32_t numberMips);
}