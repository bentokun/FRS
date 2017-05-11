#pragma once
#pragma warning (disable: 4251 4267)

#include <vector>
#include <string>
#include <algorithm>
#include <typeinfo>

#include "Buffer.h"
#include "Rules.h"
#include "Device.h"
#include "FRSCM_Reader\ShaderReader.h"
#include "FRSCM_Reader\TextureReader.h"

#ifdef _WIN32
#ifdef FRSV_EXPORTS
#define TFSAPI __declspec(dllexport)
#else
#define TFSAPI __declspec(dllimport)
#endif
#endif

namespace FRS {

	class TFSAPI ContentManager {
	public:

		ContentManager() = default;
		ContentManager(Device device, DeviceAllocator* allocator);

		friend void CreateContentManager(ContentManager* manager,
			Device device, DeviceAllocator* allocator);

		Shader* Load(std::string path, std::string path2);
		Texture* Load(std::string path);

		void Unload(Shader* para);
		void Unload(Texture* para);


	private:

		Device device;
		DeviceAllocator* allocator;
		std::vector<Shader> shaders;
		std::vector<Texture> textures;

	};



}