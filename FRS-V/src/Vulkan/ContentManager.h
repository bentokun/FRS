#pragma once

#include <vector>
#include <string>
#include <algorithm>
#include <typeinfo>

#include "Rules.h"
#include "Device.h"
#include "FRSCM_Reader\ShaderReader.h"

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
		ContentManager(Device device);

		template <class T>
		T Load(std::string path, std::string path2) {
			return T{};
		}

		Shader Load(std::string path, std::string path2){

				Shader shader = ReadModuleShader(device, path, path2);
				shaders.push_back(shader);

				return shader;
		}

		template <typename T>
		void Unload(T para) {

			const char* t = "class FRS::Shader";

			if (t == "class FRS::Shader") {

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
			else {
				std::cout << t << std::endl;
				throw std::exception("Can't destroy!");
			}
		}

	private:

		Device device;
		std::vector<Shader> shaders;

	};



}