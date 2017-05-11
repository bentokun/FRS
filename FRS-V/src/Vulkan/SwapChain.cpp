#include "SwapChain.h"

namespace FRS {

	Swapchain::Swapchain() {
		images.resize(0, { VK_NULL_HANDLE });
		imageViews.resize(0, { VK_NULL_HANDLE });
	}

	void DestroySwapchain(Swapchain* chain) {

		for (uint32_t i = 0; i < chain->imageViews.size(); i++) {
			vkDestroyImageView(chain->device.logicalDevice, chain->imageViews[i],
				nullptr);
		}

		DestroyTextureViews(&chain->depthBuffer);
		FinallyDestroyTexture(&chain->depthBuffer);

		chain->imageViews.clear();
		chain->images.clear();

		vkDestroySwapchainKHR(chain->device.logicalDevice, chain->swapChain, nullptr);

	}

	//My graphic card is burnt out
	void CreateSwapchain(Swapchain* swapChain, 
		Device device, Window window, 
		VkComponentSwizzle rgbaOption[4],
		bool vSync) {

		if (GetSwapChainSupport(device.physicalDevice) != SUCCESS) {
			throw std::runtime_error("Device dont support swap chain!");
		}

		swapChain->swapChainExtent = FRS::GetSuitableWindowExtent(device.physicalDevice, window);
		if (vSync)
			swapChain->presentMode = VK_PRESENT_MODE_FIFO_KHR;
		else
			swapChain->presentMode = FRS::GetSuitableWindowPresentMode(device.physicalDevice, window);
		
		swapChain->format = FRS::GetSuitableWindowSurfaceFormat(device.physicalDevice, window);

		uint32_t imageCount = FRS::GetMaxImageViewCount(device.physicalDevice, window);

		VkSwapchainCreateInfoKHR info = {};

		info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		info.imageExtent = swapChain->swapChainExtent;
		info.minImageCount = imageCount;
		info.imageFormat = swapChain->format.format;
		info.imageColorSpace = swapChain->format.colorSpace;
		info.presentMode = swapChain->presentMode;
		info.imageArrayLayers = 1;
		info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		info.surface = window.surface;

		QueueFamilyIndex index = FRS::findAllQueueFamily(device.physicalDevice, window);

		uint32_t queueFamily[] = { index.graphicsFamily, index.presentFamily };

		if (index.graphicsFamily != index.presentFamily) {
			info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			info.queueFamilyIndexCount = 2;
			info.pQueueFamilyIndices = queueFamily;
		}
		else {
			info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			info.queueFamilyIndexCount = 0;
			info.pQueueFamilyIndices = nullptr;
		}

		VkSurfaceCapabilitiesKHR cap = FRS::GetWindowSurfaceCapabilities(device.physicalDevice, window);

		info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		info.preTransform = cap.currentTransform;

		//Prepare for screenshot
		info.clipped = VK_FALSE;
		info.oldSwapchain = nullptr;

		VkResult result = vkCreateSwapchainKHR(device.logicalDevice, &info, nullptr, &swapChain->swapChain);
		vkGetSwapchainImagesKHR(device.logicalDevice, swapChain->swapChain, &imageCount, nullptr);
		swapChain->images.resize(imageCount);
		vkGetSwapchainImagesKHR(device.logicalDevice, swapChain->swapChain, &imageCount, swapChain->images.data());
		swapChain->imageViews.resize(imageCount);

		for (int i = 0; i < swapChain->images.size(); i++) {

			VkImageViewCreateInfo imageViewInfo = {};

			swapChain->imageViews[i] = VK_NULL_HANDLE;
			imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imageViewInfo.image = swapChain->images[i];
			imageViewInfo.components.r = rgbaOption[0];
			imageViewInfo.components.g = rgbaOption[1];
			imageViewInfo.components.b = rgbaOption[2];
			imageViewInfo.components.a = rgbaOption[3];

			imageViewInfo.format = swapChain->format.format;
			imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			imageViewInfo.subresourceRange.baseArrayLayer = 0;
			imageViewInfo.subresourceRange.layerCount = 1;
			imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageViewInfo.subresourceRange.levelCount = 1;
			imageViewInfo.subresourceRange.baseMipLevel = 0;

			VkResult nresult = vkCreateImageView(device.logicalDevice, &imageViewInfo, nullptr, &(swapChain->imageViews[i]));

		}

		swapChain->device = device;

	}

	
	Swapchain::Swapchain(Device device,
		Window window, VkComponentSwizzle rgbaOption[4],
		bool vSync) {


		if (GetSwapChainSupport(device.physicalDevice) != SUCCESS) {
			throw std::runtime_error("Device dont support swap chain!");
		}

		swapChainExtent = FRS::GetSuitableWindowExtent(device.physicalDevice, window);
		if (vSync)
			presentMode = VK_PRESENT_MODE_FIFO_KHR;
		else
			presentMode = FRS::GetSuitableWindowPresentMode(device.physicalDevice, window);

		format = FRS::GetSuitableWindowSurfaceFormat(device.physicalDevice, window);

		uint32_t imageCount = FRS::GetMaxImageViewCount(device.physicalDevice, window);

		VkSwapchainCreateInfoKHR info = {};

		info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		info.imageExtent = swapChainExtent;
		info.minImageCount = imageCount;
		info.imageFormat = format.format;
		info.imageColorSpace = format.colorSpace;
		info.presentMode = presentMode;
		info.imageArrayLayers = 1;
		info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		info.surface = window.surface;

		QueueFamilyIndex index = FRS::findAllQueueFamily(device.physicalDevice, window);

		uint32_t queueFamily[] = { index.graphicsFamily, index.presentFamily };

		if (index.graphicsFamily != index.presentFamily) {
			info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			info.queueFamilyIndexCount = 2;
			info.pQueueFamilyIndices = queueFamily;
		}
		else {
			info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			info.queueFamilyIndexCount = 0;
			info.pQueueFamilyIndices = nullptr;
		}

		VkSurfaceCapabilitiesKHR cap = FRS::GetWindowSurfaceCapabilities(device.physicalDevice, window);

		info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		info.preTransform = cap.currentTransform;

		//Prepare for screenshot
		info.clipped = VK_FALSE;
		info.oldSwapchain = nullptr;

		VkResult result = vkCreateSwapchainKHR(device.logicalDevice, &info, nullptr, &swapChain);
		vkGetSwapchainImagesKHR(device.logicalDevice, swapChain, &imageCount, nullptr);
		
		images.resize(imageCount);
		imageViews.resize(imageCount);

		vkGetSwapchainImagesKHR(device.logicalDevice, swapChain, &imageCount, images.data());

		for (int i = 0; i < images.size(); i++) {

			VkImageViewCreateInfo imageViewInfo = {};

			imageViews[i] = VK_NULL_HANDLE;
			imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imageViewInfo.image = images[i];
			imageViewInfo.components.r = rgbaOption[0];
			imageViewInfo.components.g = rgbaOption[1];
			imageViewInfo.components.b = rgbaOption[2];
			imageViewInfo.components.a = rgbaOption[3];

			imageViewInfo.format = format.format;
			imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			imageViewInfo.subresourceRange.baseArrayLayer = 0;
			imageViewInfo.subresourceRange.layerCount = 1;
			imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageViewInfo.subresourceRange.levelCount = 1;
			imageViewInfo.subresourceRange.baseMipLevel = 0;

			VkResult nresult = vkCreateImageView(device.logicalDevice, &imageViewInfo, nullptr, &(
				imageViews[i]));

		}

		this->device = device;

	}

	void RecreateSwapchain(Swapchain* swapChain,
		Window window, VkComponentSwizzle rgbaOption[4],
		bool vSync) {

#pragma region DESTRUCTION

		DestroyTextureViews(&swapChain->depthBuffer);
		FinallyDestroyTexture(&swapChain->depthBuffer);

#pragma endregion

		if (GetSwapChainSupport(swapChain->device.physicalDevice) != SUCCESS) {
			throw std::runtime_error("Device dont support swap chain!");
		}

		swapChain->swapChainExtent = FRS::GetSuitableWindowExtent(swapChain->device.physicalDevice, window);
		if (vSync)
			swapChain->presentMode = VK_PRESENT_MODE_FIFO_KHR;
		else
			swapChain->presentMode = FRS::GetSuitableWindowPresentMode(swapChain->device.physicalDevice, window);

		swapChain->format = FRS::GetSuitableWindowSurfaceFormat(swapChain->device.physicalDevice, window);

		uint32_t imageCount = FRS::GetMaxImageViewCount(swapChain->device.physicalDevice, window);

		VkSwapchainCreateInfoKHR info = {};

		info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		info.imageExtent = swapChain->swapChainExtent;
		info.minImageCount = imageCount;
		info.imageFormat = swapChain->format.format;
		info.imageColorSpace = swapChain->format.colorSpace;
		info.presentMode = swapChain->presentMode;
		info.imageArrayLayers = 1;
		info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		info.surface = window.surface;

		QueueFamilyIndex index = FRS::findAllQueueFamily(swapChain->device.physicalDevice, window);

		uint32_t queueFamily[] = { index.graphicsFamily, index.presentFamily };

		if (index.graphicsFamily != index.presentFamily) {
			info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			info.queueFamilyIndexCount = 2;
			info.pQueueFamilyIndices = queueFamily;
		}
		else {
			info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			info.queueFamilyIndexCount = 0;
			info.pQueueFamilyIndices = nullptr;
		}

		VkSurfaceCapabilitiesKHR cap = FRS::GetWindowSurfaceCapabilities(swapChain->device.physicalDevice, window);

		info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		info.preTransform = cap.currentTransform;

		//Prepare for screenshot
		info.clipped = VK_FALSE;

		swapChain->oldSwapChain = swapChain->swapChain;

		info.oldSwapchain = swapChain->oldSwapChain;

		VkSwapchainKHR newSwap;

		VkResult result = vkCreateSwapchainKHR(swapChain->device.logicalDevice, &info, nullptr, &newSwap);

		vkDestroySwapchainKHR(swapChain->device.logicalDevice, swapChain->oldSwapChain, nullptr);

		for (uint32_t i = 0; i < swapChain->imageViews.size(); i++) {
			//vkDestroyImage(chain.device->logicalDevice, chain.images[i], nullptr);
			vkDestroyImageView(swapChain->device.logicalDevice, swapChain->imageViews[i], nullptr);

		}

		swapChain->swapChain = newSwap;

		vkGetSwapchainImagesKHR(swapChain->device.logicalDevice, swapChain->swapChain, &imageCount, nullptr);

		swapChain->images.resize(imageCount);

		vkGetSwapchainImagesKHR(swapChain->device.logicalDevice, swapChain->swapChain, &imageCount, swapChain->images.data());

		swapChain->imageViews.resize(imageCount);

		for (int i = 0; i < swapChain->images.size(); i++) {

			VkImageViewCreateInfo imageViewInfo = {};

			swapChain->imageViews[i] = VK_NULL_HANDLE;
			imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imageViewInfo.image = swapChain->images[i];
			imageViewInfo.components.r = rgbaOption[0];
			imageViewInfo.components.g = rgbaOption[1];
			imageViewInfo.components.b = rgbaOption[2];
			imageViewInfo.components.a = rgbaOption[3];

			imageViewInfo.format = swapChain->format.format;
			imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			imageViewInfo.subresourceRange.baseArrayLayer = 0;
			imageViewInfo.subresourceRange.layerCount = 1;
			imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageViewInfo.subresourceRange.levelCount = 1;
			imageViewInfo.subresourceRange.baseMipLevel = 0;

			VkResult nresult = vkCreateImageView(swapChain->device.logicalDevice, &imageViewInfo, nullptr, &(swapChain->imageViews[i]));

		}


	}

}