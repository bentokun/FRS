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

		friend void Destroy(Shader* shader);

		struct UniformBinding {
			VkDeviceSize dataArrayLength = 0;

			VkDescriptorType Type;
			uint32_t *Range = new uint32_t[25];
			uint32_t *Size = new uint32_t[25];
			uint32_t *OffSet = new uint32_t[25];

			Stage stage = UNDENTIFIED;
		};

		struct UniformSet {
			UniformBinding *UniformBindings = new UniformBinding[25];
			
			void**          BindingDatas = new void*[25];
			uint32_t*       BindingSize = new uint32_t[25];

		} *UniformSets = new UniformSet[25];

		struct VertexBinding {

			struct Loc {
				VkFormat Format = { VK_FORMAT_UNDEFINED };
				VkDeviceSize Offset;
				
			} *Location = new Loc[25];
	
			uint32_t Stride;
			VkVertexInputRate InputRate;
		};

		struct VertexInput {
			VertexBinding *VertexBindings = new VertexBinding[25];
			void**        BindingDatas = new void*[25];
			uint32_t*      BindingSize = new uint32_t[25];

			uint32_t numbersBindings;
			uint32_t numberDatas;
		} VertexInput;

		struct IndexInput {
			void** IndexDatas = new void*[25];
			uint32_t* IndexSize = new uint32_t[25];
		} IndexInput;

#pragma region OPERATOR
		bool operator == (Shader shader);
		bool operator == (Shader* shader);
#pragma endregion

#pragma region GETTER
		VkShaderModule& GetVertexModule() {
			return vertexModule;
		}

		VkShaderModule& GetFragmentModule() {
			return fragModule;
		}
#pragma endregion
	private:

		Device device;
		VkShaderModule vertexModule, fragModule;

	};

	TFSAPI void GetShaderBind(Shader* shader, std::vector<VkVertexInputAttributeDescription>& attributeDes,
		std::vector<VkVertexInputBindingDescription>& bindingsDes);

}