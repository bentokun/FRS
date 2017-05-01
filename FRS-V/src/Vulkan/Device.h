#pragma once
#pragma warning (disable: 4251 4267)

#include "Window.h"
#include "VKExtensions.h"

#ifdef _WIN32
#ifdef FRSV_EXPORTS
#define TFSAPI __declspec(dllexport)
#else
#define TFSAPI __declspec(dllimport)
#endif
#endif

namespace FRS {

	//Should only appeared once in a Game class
	struct TFSAPI Device {

	public:

		Device() {};
		~Device() {};

		Device(Device const &device) {
			this->logicalDevice = device.logicalDevice;
			this->physicalDevice = device.physicalDevice;
			this->index = device.index;
		}

		Device(VkInstance instance, Window window, bool validationLayer);
		friend void CreateDevice(Device* device, VkInstance instance, Window window, bool validationLayer);

		bool operator == (Device para) {
			if (physicalDevice == para.physicalDevice &&
				logicalDevice == para.logicalDevice &&
				index == para.index)

				return true;

			return false;
		}

		VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };
		VkDevice logicalDevice{ VK_NULL_HANDLE };
	
		uint32_t GetPresentFamily();
		uint32_t GetGraphicFamily();

		friend void DestroyDevice(Device device);


	private:

		QueueFamilyIndex index;
	};

	

}