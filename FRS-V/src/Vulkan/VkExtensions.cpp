#include "VkExtensions.h"

namespace FRS {

	std::vector<std::string> vkInstanceExtensions() {

		uint32_t totalExtensions;

		vkEnumerateInstanceExtensionProperties(nullptr, &totalExtensions, nullptr);

		std::vector<VkExtensionProperties> extProps{ totalExtensions };
		std::vector<std::string> extensions{ totalExtensions };

		vkEnumerateInstanceExtensionProperties(nullptr, &totalExtensions, extProps.data());

		for (int i = 0; i < totalExtensions; i++) {
			extensions[i] = extProps[i].extensionName;
		}

		return extensions;

	}

	FRS_STATE vkCheckValidation() {

		std::vector<const char*> layerCheck{
			"VK_LAYER_LUNARG_standard_validation"
		};

		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> properties(layerCount, { VK_NULL_HANDLE });
		vkEnumerateInstanceLayerProperties(&layerCount, properties.data());

		for (auto& layer : layerCheck) {

			bool layerFound = false;

			for (auto& layerProperty : properties) {
				if (strcmp(layerProperty.layerName, layer)) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound) {
				std::cout << "Cant find validation lauer!" << std::endl;
				return FAILURE;
			}
				
		}

		return SUCCESS;


	}

	VkResult CreateDebugReportCallback(VkInstance instance,
		const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugReportCallbackEXT* pCallback) {

		auto func = (PFN_vkCreateDebugReportCallbackEXT)
			vkGetInstanceProcAddr(instance,
				"vkCreateDebugReportCallbackEXT");

		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pCallback);
		}
		else
			return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	void DestroyDebugReportCallbackEXT(VkInstance instance,
		VkDebugReportCallbackEXT callback,
		const VkAllocationCallbacks* pAllocator) {

		auto func = (PFN_vkDestroyDebugReportCallbackEXT)
			vkGetInstanceProcAddr(instance,
				"vkDestroyDebugReportCallbackEXT");

		if (func != nullptr) {
			func(instance, callback, pAllocator);
		}

	}

	FRS_STATE vkEnabledConsoleReport(VkInstance instance,
		PFN_vkDebugReportCallbackEXT callbackFunc,
	    VkDebugReportCallbackEXT* reportext) {

		VkDebugReportCallbackCreateInfoEXT reportInfo;

		reportInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		reportInfo.pfnCallback = callbackFunc;
		reportInfo.flags = VK_DEBUG_REPORT_DEBUG_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
		reportInfo.pUserData = nullptr;

		if (FRS::CreateDebugReportCallback(instance, &reportInfo,
			nullptr, reportext) != VK_SUCCESS) {

			std::cout << "Can't Create Report!" << std::endl;
			return FAILURE;

		}


		return SUCCESS;

	}

	FRS_STATE ChoosePhysicalDevice(VkInstance instance, VkPhysicalDevice& device) {

		uint32_t physicalDeviceCounts;
		vkEnumeratePhysicalDevices(instance, &physicalDeviceCounts, nullptr);

		std::cout << "\n";
		std::cout << "Total GPUs: " << physicalDeviceCounts << std::endl;

		if (physicalDeviceCounts == 0) {
			FRS_FATAL_ERROR("Where is your GPU. Dont play games!");
		}

		std::vector<VkPhysicalDevice> devices{ physicalDeviceCounts };
		vkEnumeratePhysicalDevices(instance, &physicalDeviceCounts, devices.data());

		//Auto-ordered map
		std::multimap<int, VkPhysicalDevice> deviceScoreMaps;

		if (devices.size() == 1) {
			device = devices[0];
			return SUCCESS;
		}

		for (const auto& pdevice : devices) {
			int score = PhysicalDeviceScore(pdevice);

			deviceScoreMaps.insert(std::make_pair(score, pdevice));
		}

		if (deviceScoreMaps.size() == 1) {
			device = deviceScoreMaps.begin()->second;
			std::cout << "\t" << "\t" << "GPU choosen score: " << deviceScoreMaps.begin()->first << std::endl;
			return SUCCESS;
		}

		if (deviceScoreMaps.rbegin()->first > 0) {

			QueueFamilyIndex index = findAllQueueFamily(deviceScoreMaps.rbegin()->second, nullptr);
			bool compitableQueue = index.isGraphicComplete();

			if (compitableQueue)
				device = deviceScoreMaps.rbegin()->second;

		}
		else {
			return FAILURE;
		}

		return SUCCESS;

	}


	int PhysicalDeviceScore(VkPhysicalDevice device) {
		int score = 0;

		
		VkPhysicalDeviceProperties props;
		VkPhysicalDeviceFeatures feautures;

		vkGetPhysicalDeviceProperties(device, &props);
		vkGetPhysicalDeviceFeatures(device, &feautures);

		if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			score += 1000;

		//Rank GPU based on the vendor
		switch (props.deviceType)
		{
			//NVIDIA
		case 0x10DE:
			std::cout << "NVIDIA!" << std::endl;
			score += 1000;
			break;

			//AMD
		case 0x1002:
			score += 800;
			break;

			//VR-GPU
		case 0x1010:
			score += 1000;
			break;

			//Intel.
		case 0x8086:
			score += 600;
			break;

			//ARM
		case 0x13B5:
			score += 600;
			break;

			//qualcomm is greater than arm
			//That's my thought on using things on WM10
		case 0x5143:
			score += 800;
			break;

		default:
			break;
		}

		score += props.limits.maxImageDimension2D;

		//Throw the fucking GPU away if dont support geometryShader
		if (!feautures.geometryShader) {
			return 0;
		}

		//If support 64 bit float and int, it's great af
		if (feautures.shaderFloat64 && feautures.shaderInt64) {
			score += 500;
		}

		//Needed for my game
		if (feautures.occlusionQueryPrecise) {
			score += 700;
		}

		return score;
	}

	FRS_STATE CreateLogicalDevice(VkPhysicalDevice device,
		FRS::Window window,
		VkDevice* logicalDevice,
		bool validationLayer) {

		VkDeviceCreateInfo deviceInfo = {};

		QueueFamilyIndex index = FRS::findAllQueueFamily(device, &window);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<int> family = { index.graphicsFamily, index.presentFamily };

		float queuePriority = 1.0f;

		for (int queueFam : family) {
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.flags = 0;
			queueCreateInfo.pNext = NULL;
			queueCreateInfo.queueFamilyIndex = queueFam;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		std::vector<const char*> layerCheck{
			"VK_LAYER_LUNARG_standard_validation"
		};

		std::vector<std::string> extensionNamesSTD = FRS::GetDeviceExtensionSupport(device);
		std::vector<const char*> extensionNames{ extensionNamesSTD.size() };

		if (extensionNamesSTD.size() == 0) {
			FRS_FATAL_ERROR("Extension cant be founded");
		}

		for (int i = 0; i < extensionNames.size(); i++) {
			extensionNames[i] = extensionNamesSTD[i].c_str();
		}
		
		VkPhysicalDeviceFeatures feautures = {};

		deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceInfo.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();
		deviceInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceInfo.pNext = NULL;
		deviceInfo.flags = 0;
		deviceInfo.enabledExtensionCount = extensionNames.size();
		deviceInfo.ppEnabledExtensionNames = extensionNames.data();
		deviceInfo.pEnabledFeatures = &feautures;

		if (validationLayer) {
			deviceInfo.enabledLayerCount = layerCheck.size();
			deviceInfo.ppEnabledLayerNames = layerCheck.data();
		}
		else {
			deviceInfo.enabledLayerCount = 0;
		}

		if (logicalDevice != VK_NULL_HANDLE) {
			vkDestroyDevice(*logicalDevice, nullptr);
		}

		VkResult result = vkCreateDevice(device, &deviceInfo, nullptr, logicalDevice);

		if (result!= VK_SUCCESS){
			std::cout << result << std::endl;
			FRS_FATAL_ERROR("Cant create Vulkan device!");
			return FAILURE;
		}

		return SUCCESS;
	}

	void CreateQueue(VkDevice device, VkPhysicalDevice physicalDevice,
		FRS::Window& window, VkQueue* graphicFam, VkQueue* presentFam) {


		QueueFamilyIndex index = findAllQueueFamily(physicalDevice, &window);

		vkGetDeviceQueue(device, index.graphicsFamily, 0, graphicFam);
		vkGetDeviceQueue(device, index.presentFamily, 0, presentFam);

	}

	std::vector<std::string> GetDeviceExtensionSupport(VkPhysicalDevice device) {

		uint32_t extensionCounts;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCounts, nullptr);

		std::vector<VkExtensionProperties> extensions{ extensionCounts };
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCounts, extensions.data());

		std::vector<std::string> extensionName;

		for (auto& extension : extensions) {
			extensionName.push_back(extension.extensionName);
		}

		return extensionName;

	}

	FRS_STATE GetSwapChainSupport(VkPhysicalDevice device) {

		std::vector<std::string> extensionNames = FRS::GetDeviceExtensionSupport(device);

		std::vector<std::string> swapChainName{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		std::set<std::string> extensionSet{ swapChainName.begin(), swapChainName.end() };

		for (auto& extensionName : extensionNames) {
			extensionSet.erase(extensionName);
		}

		return (FRS_STATE)(extensionSet.empty());

	}

	VkSurfaceCapabilitiesKHR GetWindowSurfaceCapabilities(VkPhysicalDevice device, Window* window) {

		VkSurfaceCapabilitiesKHR capabilities {VK_NULL_HANDLE};

		if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, window->surface, &capabilities) != VK_SUCCESS) {
			FRS_FATAL_ERROR("Cant get surface caps");
		}

		return capabilities;
	}

	VkExtent2D GetSuitableWindowExtent(VkPhysicalDevice device, Window* window) {

		VkSurfaceCapabilitiesKHR capabilities = GetWindowSurfaceCapabilities(device, window);

		if (capabilities.currentExtent.width != reinterpret_cast<uint32_t>(std::numeric_limits<uint32_t>::max)) {
			return capabilities.currentExtent;
		}
		else {
			VkExtent2D extent = window->GetWindowExtent();

			extent.width = FRSML::Clamp(capabilities.minImageExtent.width,
				capabilities.maxImageExtent.width,
				extent.width);

			extent.height = FRSML::Clamp(capabilities.minImageExtent.height,
				capabilities.maxImageExtent.height,
				extent.height);

			return extent;
		}

	


	}


	VkSurfaceFormatKHR GetSuitableWindowSurfaceFormat(VkPhysicalDevice device, Window* window) {

		uint32_t surfaceFormatCounts;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, window->surface, &surfaceFormatCounts, nullptr);

		std::vector<VkSurfaceFormatKHR> surfaceFormats{ surfaceFormatCounts };
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, window->surface, &surfaceFormatCounts, surfaceFormats.data());

		if (surfaceFormats.size() == 0) {
			FRS_FATAL_ERROR("Can't choose suitable surface format!");
		}

		if (surfaceFormats.size() == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED) {
			return { VK_FORMAT_B8G8R8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
		}

		for (VkSurfaceFormatKHR surface : surfaceFormats) {
			if (surface.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR
				&& surface.format == VK_FORMAT_R8G8B8A8_UNORM)
				return surface;
		}

		return surfaceFormats[0];

	}

	VkPresentModeKHR GetSuitableWindowPresentMode(VkPhysicalDevice device, Window* window) {

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, window->surface, &presentModeCount, nullptr);

		std::vector<VkPresentModeKHR> presentModes{ presentModeCount };
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, window->surface, &presentModeCount, presentModes.data());

		if (presentModeCount == 0) {
			FRS_FATAL_ERROR("Can't choose suitable presentmode!");
		}

		for (VkPresentModeKHR presentMode : presentModes) {
			if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
				return presentMode;
		}

		return VK_PRESENT_MODE_FIFO_KHR;

	}


	TFAPI uint32_t GetMaxImageViewCount(VkPhysicalDevice device, Window* window) {

		VkSurfaceCapabilitiesKHR capabilities = GetWindowSurfaceCapabilities(device, window);
		
		uint32_t ImageCount = capabilities.minImageCount + 1;

		if (capabilities.maxImageCount > 0 &&
			ImageCount > capabilities.maxImageCount) {

			ImageCount = capabilities.maxImageCount;

		}

		return ImageCount;

	}

}
