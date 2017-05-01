#pragma once
#pragma warning (disable: 4251 4267)

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

		struct UniformBinding {
			VkDeviceSize dataArrayLength = 0;
			Stage stage = UNDENTIFIED;
		};

		struct UniformSet {
			UniformBinding UniformBindings[25];
			void*          BindingDatas[25]{ nullptr };
			uint32_t       BindingSize[25] {0};

		} UniformSets[25];

		struct VertexBinding {

			struct Loc {
				VkFormat Format = { VK_FORMAT_UNDEFINED };
				VkDeviceSize Offset;
			} Location[25];
	
			uint32_t Stride;
			VkVertexInputRate InputRate;
		};

		struct VertexInput {
			VertexBinding VertexBindings[25]{};
			void*         BindingDatas[25]{ nullptr };
			uint32_t      BindingSize[25]{ 0 };

			uint32_t numbersBindings;
			uint32_t numberDatas;
		} VertexInput;

		struct IndexInput {
			void* IndexDatas[25]{ nullptr };
			uint32_t IndexSize[25]{ 0 };
		} IndexInput;

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


	TFAPI void GetShaderBind(Shader shader, std::vector<VkVertexInputAttributeDescription>& attributeDes,
		std::vector<VkVertexInputBindingDescription>& bindingsDes);

}