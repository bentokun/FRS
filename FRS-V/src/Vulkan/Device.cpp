#include "Device.h"

namespace FRS {

	Device::Device(VkInstance instance, Window window, bool validationLayer) {
		ChoosePhysicalDevice(instance, physicalDevice);
		CreateLogicalDevice(physicalDevice, window, &logicalDevice, validationLayer);

		index = FRS::findAllQueueFamily(physicalDevice, window);
	};

	void CreateDevice(Device* device, VkInstance instance, Window window, bool validationLayer) {
		ChoosePhysicalDevice(instance, device->physicalDevice);
		CreateLogicalDevice(device->physicalDevice, window, &device->logicalDevice, validationLayer);

		device->index = FRS::findAllQueueFamily(device->physicalDevice, window);
	}

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