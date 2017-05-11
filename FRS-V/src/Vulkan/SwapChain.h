#pragma once
#pragma warning (disable: 4251 4267)

#include "Rules.h"
#include "Window.h"
#include "Device.h"
#include "VkExtensions.h"
#include "Texture.h"

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
		Swapchain(Device device, Window window,
			VkComponentSwizzle rgbaOption[4], bool vSync = false);

		friend void CreateSwapchain(Swapchain* swapChain, Device device, Window window,
			VkComponentSwizzle rgbaOption[4], bool vSync = false);

		VkSurfaceFormatKHR format;
		VkPresentModeKHR presentMode;

		VkExtent2D swapChainExtent{ VK_NULL_HANDLE };

		friend void DestroySwapchain(Swapchain* chain);

		friend void RecreateSwapchain(Swapchain* swapChain,
			Window window, VkComponentSwizzle rgbaOption[4],
		    bool vSync = false);

		std::vector<VkImage> images;
		std::vector<VkImageView> imageViews;

		VkSwapchainKHR swapChain{ VK_NULL_HANDLE };
		VkSwapchainKHR oldSwapChain{ VK_NULL_HANDLE };

		Texture depthBuffer;

	private:
		
		Device device;

	};

}