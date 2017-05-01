#pragma once

#include <string>

#include "../Texture.h"
#include "../../Img/DDS.h"



#ifdef _WIN32
#ifdef FRSV_EXPORTS
#define TFSAPI __declspec(dllexport)
#else
#define TFSAPI __declspec(dllimport)
#endif
#endif

namespace FRS {

	Texture TFSAPI ReadTexture(Device device, DeviceAllocator allocator,
		TextureType type,
		std::string path);

}