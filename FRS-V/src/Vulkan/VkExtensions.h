#pragma once

#include <vector>
#include <map>
#include <set>
#include <algorithm>

#include <Windows.h>

#include <FRSML>

#include "Window.h"
#include "Rules.h"
#include "Assert.h"
#include "QueueFamily.h"

#ifdef _WIN32
#ifdef FRSV_EXPORTS
#define TFAPI __declspec(dllexport)
#else
#define TFAPI __declspec(dllimport)
#endif
#endif

namespace FRS {

	TFAPI std::vector<std::string> vkInstanceExtensions();
	TFAPI FRS_STATE vkCheckValidation();

	TFAPI VkResult CreateDebugReportCallback(VkInstance instance,
		const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugReportCallbackEXT* pCallback);

	TFAPI void DestroyDebugReportCallbackEXT(VkInstance instance,
		VkDebugReportCallbackEXT callback,
		const VkAllocationCallbacks* pAllocator);

	TFAPI FRS_STATE vkEnabledConsoleReport(VkInstance instance,
		PFN_vkDebugReportCallbackEXT callbackFunc,
		VkDebugReportCallbackEXT* reportext);

	//Choose between physicalDevice had in the computer
	TFAPI FRS_STATE ChoosePhysicalDevice(VkInstance instance, VkPhysicalDevice& device);
	TFAPI FRS_STATE CreateLogicalDevice(VkPhysicalDevice device, FRS::Window window,
		 VkDevice* logicalDevice,
		 bool validationLayer);

	TFAPI VkResult CreateWin32SurfaceKHR(VkInstance instance,
		VkWin32SurfaceCreateInfoKHR* surfaceInfo,
		VkAllocationCallbacks* callback,
		VkSurfaceKHR* surface);

	TFAPI FRS_STATE CreateSurface(FRS::Window* window, VkInstance instance);
	TFAPI void CreateQueue(VkDevice device, VkPhysicalDevice physicalDevice,
		FRS::Window& window,	VkQueue* graphicFam, VkQueue* presentFam);

	TFAPI std::vector<std::string> GetDeviceExtensionSupport(VkPhysicalDevice device);
	TFAPI FRS_STATE GetSwapChainSupport(VkPhysicalDevice device);

	TFAPI VkSurfaceCapabilitiesKHR GetWindowSurfaceCapabilities(VkPhysicalDevice device, Window* window);
	
	TFAPI VkExtent2D GetSuitableWindowExtent(VkPhysicalDevice device, Window* window);
	TFAPI VkSurfaceFormatKHR GetSuitableWindowSurfaceFormat(VkPhysicalDevice device, Window* window);

	TFAPI VkPresentModeKHR GetSuitableWindowPresentMode(VkPhysicalDevice device, Window* window);

	TFAPI uint32_t GetMaxImageViewCount(VkPhysicalDevice device, Window* window);

	//Rate the score for a physical device
	TFAPI int PhysicalDeviceScore(VkPhysicalDevice device);

}