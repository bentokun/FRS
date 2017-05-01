#include "QueueFamily.h"

namespace FRS {

	QueueFamilyIndex findAllQueueFamily(VkPhysicalDevice device,
		FRS::Window window) {

		QueueFamilyIndex index;

		uint32_t queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueProperties{ queueFamilyCount };
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueProperties.data());

		int i = 0;

		//For all of the queue family, find one that support graphics queue
		for (auto& queueProperty : queueProperties) {
			if (queueProperty.queueCount > 0 && queueProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				index.graphicsFamily = i;
			}

			if (index.isGraphicComplete()) {
				break;
			}

			i++;
		};

		i = 0;

		for (auto& queueProperty : queueProperties) {
			VkBool32 presentSupport = false;

			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, window.surface, &presentSupport);

			if (queueProperty.queueCount > 0 && presentSupport) {
				index.presentFamily = i;
			}

			if (index.isPresentComplete()) {
				break;
			}

			i++;
		}
	
	

		return index;
	}

}
