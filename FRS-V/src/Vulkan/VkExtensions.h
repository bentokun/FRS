#pragma once
#pragma warning (disable: 4251 4267)

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
#define TFSAPI __declspec(dllexport)
#else
#define TFSAPI __declspec(dllimport)
#endif
#endif

namespace FRS {

	TFSAPI std::vector<std::string> vkInstanceExtensions();
	TFSAPI FRS_STATE vkCheckValidation();

	TFSAPI void CreateInstance(std::string appName, bool EnableValidation, VkInstance* instance);

	TFSAPI VkResult CreateDebugReportCallback(VkInstance instance,
		const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugReportCallbackEXT* pCallback);

	TFSAPI void DestroyDebugReportCallbackEXT(VkInstance instance,
		VkDebugReportCallbackEXT callback,
		const VkAllocationCallbacks* pAllocator);

	TFSAPI FRS_STATE vkEnabledConsoleReport(VkInstance instance,
		PFN_vkDebugReportCallbackEXT callbackFunc,
		VkDebugReportCallbackEXT* reportext);

	//Choose between physicalDevice had in the computer
	TFSAPI FRS_STATE ChoosePhysicalDevice(VkInstance instance, VkPhysicalDevice& device);
	TFSAPI FRS_STATE CreateLogicalDevice(VkPhysicalDevice device, FRS::Window window,
		 VkDevice* logicalDevice,
		 bool validationLayer);

	TFSAPI std::vector<std::string> GetDeviceExtensionSupport(VkPhysicalDevice device);
	TFSAPI FRS_STATE GetSwapChainSupport(VkPhysicalDevice device);

	TFSAPI VkSurfaceCapabilitiesKHR GetWindowSurfaceCapabilities(VkPhysicalDevice device, Window window);
	
	TFSAPI VkExtent2D GetSuitableWindowExtent(VkPhysicalDevice device, Window window);
	TFSAPI VkSurfaceFormatKHR GetSuitableWindowSurfaceFormat(VkPhysicalDevice device, Window window);

	TFSAPI VkPresentModeKHR GetSuitableWindowPresentMode(VkPhysicalDevice device, Window window);

	TFSAPI uint32_t GetMaxImageViewCount(VkPhysicalDevice device, Window window);

	//Rate the score for a physical device
	TFSAPI int PhysicalDeviceScore(VkPhysicalDevice device);

}