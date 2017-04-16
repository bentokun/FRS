#include "Device.h"

namespace FRS {

	Device::Device(VkInstance instance, Window* window, bool validationLayer) {

		ChoosePhysicalDevice(instance, physicalDevice);
		CreateLogicalDevice(physicalDevice, *window, &logicalDevice, validationLayer);

		index = FRS::findAllQueueFamily(physicalDevice, window);

	};

	uint32_t Device::GetPresentFamily() {
		return index.presentFamily;
	}

	uint32_t Device::GetGraphicFamily() {
		return index.graphicsFamily;
	}

	void DestroyDevice(Device device) {
		vkDestroyDevice(device.logicalDevice, nullptr);
	}

}