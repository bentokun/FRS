#pragma once

#include "Rules.h"
#include "Window.h"
#include "Device.h"
#include "VkExtensions.h"

#ifdef _WIN32
#ifdef FRSV_EXPORTS
#define TFSAPI __declspec(dllexport)
#else
#define TFSAPI __declspec(dllimport)
#endif
#endif

namespace FRS {

	class TFSAPI Swapchain {

	public:

		Swapchain();

		//In case we want monochrome effect, cause loading image is easier than doing shading
		//if we can load image :)).
		Swapchain(Device* device, Window* window,
			VkComponentSwizzle rgbaOption[4] );

		VkSurfaceFormatKHR format;
		VkPresentModeKHR presentMode;

		VkExtent2D swapChainExtent{ VK_NULL_HANDLE };

		friend void DestroyTheSwapchain(Swapchain chain);

		std::vector<VkImage> images;
		std::vector<VkImageView> imageViews;

		VkSwapchainKHR swapChain{ VK_NULL_HANDLE };
		VkSwapchainKHR oldSwapChain{ VK_NULL_HANDLE };

		friend void RecreateTheSwapchain(Swapchain* swapChain,
			Window* window, VkComponentSwizzle rgbaOption[4]);


	private:
		
		Device* device;

	};

	
	

}