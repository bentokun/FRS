#pragma once

#include <functional>
#include <fstream>

#include "Window.h"
#include "Device.h"
#include "VKExtensions.h"
#include "SwapChain.h"
#include "GraphicPipeline.h"
#include "Commander.h"
#include "ContentManager.h"

#ifdef _WIN32
#ifdef FRSV_EXPORTS
#define TFAPI __declspec(dllexport)
#else
#define TFAPI __declspec(dllimport)
#endif
#endif

template class TFAPI std::function<void(std::string)>;
typedef std::function<void(std::string)> FRSDebug_CallbackFunc;
typedef VkComponentSwizzle FRSImageComponent;

namespace FRS {

	//Base class
	//Available component:
	//	- A window
	//  - An instance
	//  - An logical device, physical device
	//  - A reporter (validiation layer)
	//  - A Swap Chain

	class TFAPI Game{
	public:

		Game() {};
		Game(std::string appName, bool debugMessageEnabled);

		~Game();

		void Run();

		void Start();
		void Load();
		void Update();
		void Draw();
		void CleanUp();
		void Unload();
		void Recreate();
		void ResizingHandler(int width, int height);

		void CreateGameDevice(Device* device, Window window);
		void DisableConsoleCallback();
		void DestroySwapChain(Device device, Swapchain swapChain);
		
		FRS_STATE EnabledConsoleCallback(PFN_vkDebugReportCallbackEXT func);
		FRS_STATE CreateSwapChain(Swapchain* swapChain, Device* device, 
			FRSImageComponent rbgacomponent[4], Window* window);

	private:

		bool EnableValidation;

		bool quit;

		Buffer buffer1{ };
		Window window;
		DeviceAllocator allocator;

		VkInstance instance = VK_NULL_HANDLE;

		ContentManager manager;
		Shader shader;

		Device device;
		static FRSDebug_CallbackFunc callbackFunc;
		Reporter reporter = VK_NULL_HANDLE;
		Swapchain swapChain;

		GraphicPipeline graphPipeline;
		Commander commander;


		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallbackFunc(
			VkDebugReportFlagsEXT flags,
			VkDebugReportObjectTypeEXT objType,
			uint64_t obj,
			size_t location,
			int32_t code,
			const char* layerPrefix,
			const char* msg,
			void* userData) {

			std::cout<<msg<<std::endl;
			return VK_FALSE;
		}

	};

}



