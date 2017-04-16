#pragma once

#include <vector>

#include "Rules.h"
#include "Window.h"

#ifdef _WIN32
#ifdef FRSV_EXPORTS
#define TFAPI __declspec(dllexport)
#else
#define TFAPI __declspec(dllimport)
#endif
#endif

namespace FRS {

	struct TFAPI QueueFamilyIndex {
		int graphicsFamily = -1;
		int presentFamily = -1;

		bool isGraphicComplete(){
			return graphicsFamily >= 0;
		}

		bool isPresentComplete() {
			return presentFamily >= 0;
		}

		bool operator ==(QueueFamilyIndex ant) {
			if (graphicsFamily == ant.graphicsFamily &&
				presentFamily == ant.presentFamily)
				return true;

			return false;
		}
	};

	TFAPI QueueFamilyIndex findAllQueueFamily(VkPhysicalDevice device,
		FRS::Window* window);

}