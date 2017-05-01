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
//#include "FRSCM_Reader\TextureReader.h"

#ifdef _WIN32
#ifdef FRSV_EXPORTS
#define TFAPI __declspec(dllexport)
#else
#define TFAPI __declspec(dllimport)
#endif
#endif

namespace FRS {

	class TFAPI ContentManager {
	public:

		ContentManager() = default;
		ContentManager(Device device, DeviceAllocator allocator);

		friend void CreateContentManager(ContentManager* manager,
			Device device, DeviceAllocator allocator);

		Shader Load(std::string path, std::string path2) {

			Shader shader = ReadModuleShader(device, path, path2);
			shaders.push_back(shader);

			return shader;
		}

		/*
		Texture Load(std::string path) {

			Texture texture = ReadTexture(device, allocator,
				TEXTURE_2D, path);

			textures.push_back(texture);
			return texture;

		}*/


		void Unload(Shader para) {

			auto shaderToFind = std::find(
				shaders.begin(), shaders.end(), para
			);

			if (shaderToFind != shaders.end()) {
				std::cout << "Destroyed!" << std::endl;
				Destroy(para);
				shaders.erase(shaderToFind);
			}
			else {
				std::cout << "Haven't destroy shader!" << std::endl;
			}

		}

		/*
		void Unload(Texture para) {
			auto textureToFind = std::find(
				textures.begin(), textures.end(), para
			);

			if (textureToFind != textures.end()) {
				std::cout << "Destroyed!" << std::endl;
				Destroy(para);
				textures.erase(textureToFind);
			}
			else {
				std::cout << "Haven't destroy shader!" << std::endl;
			}
		}*/

	private:

		Device device;
		DeviceAllocator allocator;
		std::vector<Shader> shaders;
		//std::vector<Texture> textures;

	};



}