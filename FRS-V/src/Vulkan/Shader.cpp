#include "Shader.h"

namespace FRS {

	Shader::Shader(Device device, 
		std::vector<char> vertexCode, 
		std::vector<char> fragCode,
		std::string path1, std::string path2) {
		
		this->device = device;

		VkShaderModuleCreateInfo vCreateInfo = {};

		vCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

		std::vector<uint32_t> codeAligned(vertexCode.size() / sizeof(uint32_t) + 1);
		memcpy(codeAligned.data(), vertexCode.data(), vertexCode.size());
		
		vCreateInfo.codeSize = vertexCode.size();
		vCreateInfo.pCode = codeAligned.data();

		if (vkCreateShaderModule(device.logicalDevice, &vCreateInfo, nullptr, &vertexModule) != VK_SUCCESS) {
			FRS_MESSAGE("Cant create shader module");
		}

		VkShaderModuleCreateInfo fCreateInfo = {};

		std::vector<uint32_t> codeAligned2(fragCode.size() / sizeof(uint32_t) + 1);
		memcpy(codeAligned2.data(), fragCode.data(), fragCode.size());

		fCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		fCreateInfo.codeSize = fragCode.size();
		fCreateInfo.pCode = codeAligned2.data();

		if (vkCreateShaderModule(device.logicalDevice, &fCreateInfo, nullptr, &fragModule) != VK_SUCCESS) {
			FRS_MESSAGE("Cant create shader module");
		}

	}

	void GetShaderBind(Shader shader, std::vector<VkVertexInputAttributeDescription>& attributeDes,
		std::vector<VkVertexInputBindingDescription>& bindingsDes) {

		std::vector<VkVertexInputAttributeDescription> attributesDesc;
		std::vector<VkVertexInputBindingDescription> bindingDes;

		for (uint32_t i = 0; i < 25; i++) {
			if (shader.VertexInput.VertexBindings[i].Location[0].Format == VK_FORMAT_UNDEFINED) {
				break;
			}
			else {

				VkVertexInputBindingDescription inputDes;
				inputDes.binding = i;
				inputDes.inputRate = shader.VertexInput.VertexBindings[i].InputRate;
				inputDes.stride = shader.VertexInput.VertexBindings[i].Stride;

				bindingDes.push_back(inputDes);

				for (uint32_t j = 0; j < 25; j++) {
					if (shader.VertexInput.VertexBindings[i].Location[j].Format == VK_FORMAT_UNDEFINED) {
						break;
					}
					else {
						VkVertexInputAttributeDescription attributeDes = {};
						attributeDes.offset = shader.VertexInput.VertexBindings[i].Location[j].Offset;
						attributeDes.location = j;
						attributeDes.binding = i;
						attributeDes.format = shader.VertexInput.VertexBindings[i].Location[j].Format;

						attributesDesc.push_back(attributeDes);

					}
				}
			}
		}

		attributeDes = attributesDesc;
		bindingsDes = bindingDes;
	}

}