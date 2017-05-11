#include "ContentManager.h"

namespace FRS {

	ContentManager::ContentManager(Device device,
		DeviceAllocator* allocator)
		:device(device),
		allocator(allocator){

	}

	void CreateContentManager(ContentManager* manager, Device device,
		DeviceAllocator* allocator) {
		manager->device = device;
		manager->allocator = allocator;
	}

	Shader* ContentManager::Load(std::string path, std::string path2) {

		Shader shader = ReadModuleShader(device, path, path2);
		shaders.push_back(shader);

		return &shaders[shaders.size() - 1];
	}

	Texture* ContentManager::Load(std::string path) {

		Texture texture = ReadTexture(device, allocator,
			TEXTURE_2D, path);

		textures.push_back(texture);
		return &textures[textures.size() - 1];

	}
	
	void ContentManager::Unload(Shader* para) {

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

	void ContentManager::Unload(Texture* para) {

		auto textureToFind = std::find(
			textures.begin(), textures.end(), para
		);

		if (textureToFind != textures.end()) {
			std::cout << "Destroyed!" << std::endl;
			Destroy(*para);
			textures.erase(textureToFind);
		}
		else {
			std::cout << "Haven't destroy shader!" << std::endl;
		}

	}

}