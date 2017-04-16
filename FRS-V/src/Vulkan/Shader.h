#pragma once

#include <string>

#include "Rules.h"
#include "Assert.h"
#include "Device.h"

#ifdef _WIN32
#ifdef FRSV_EXPORTS
#define TFSAPI __declspec(dllexport)
#else
#define TFSAPI __declspec(dllimport)
#endif
#endif

namespace FRS {

	//Support parsing

	class TFSAPI Shader {
	public:

		Shader() = default;

		Shader(Device device, 
			std::vector<char> vertexCode,
			std::vector<char> fragCode,
			std::string name1,
			std::string name2);

		friend void Destroy(Shader shader) {
			vkDestroyShaderModule(shader.device.logicalDevice,
				shader.vertexModule, nullptr);
			vkDestroyShaderModule(shader.device.logicalDevice,
				shader.fragModule, nullptr);
		}

		bool operator == (Shader shader) {

			if (vertexModule == shader.vertexModule &&
				fragModule == shader.fragModule &&
				device == shader.device)

				return true;

			return false;
		}

		VkShaderModule& GetVertexModule() {
			return vertexModule;
		}

		VkShaderModule& GetFragmentModule() {
			return fragModule;
		}

	private:

		Device device;

		VkShaderModule vertexModule, fragModule;
	};


}