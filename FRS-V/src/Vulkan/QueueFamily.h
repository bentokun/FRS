#pragma once
#pragma warning (disable: 4251 4267)

#include <vector>

#include "Rules.h"
#include "Window.h"

#ifdef _WIN32
#ifdef FRSV_EXPORTS
#define TFSAPI __declspec(dllexport)
#else
#define TFSAPI __declspec(dllimport)
#endif
#endif

namespace FRS {

	struct TFSAPI QueueFamilyIndex {
		int graphicsFamily = -1;
		int presentFamily = -1;

		bool isGraphicComplete(){
			return graphicsFamily >= 0;
		}

		bool isPresentComplete() {
			return presentFamily >= 0;
		}

		bool operator ==(QueueFamilyIndex ant);
	};

	TFSAPI QueueFamilyIndex findAllQueueFamily(VkPhysicalDevice device,
		FRS::Window window);

}