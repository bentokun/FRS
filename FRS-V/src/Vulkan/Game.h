#pragma once
#pragma warning (disable: 4251 4267)

#include <functional>
#include <fstream>
#include <chrono>
#include <thread>

#include "../Input/Controller.h"
#include "../../Time.h"
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


	struct UniformBufferObject {
		FRSML::mat4 model, view, proj;
	};

	struct Color {
		float sinTime;
	};

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
		Game(std::string appName, bool debugMessageEnabled,
			FRSWindowState state, int windowWidth, int  windowHeight);

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
		void InputHandler(int code, FRSKeyState state);

		void DisableConsoleCallback();
		
		FRS_STATE EnabledConsoleCallback(PFN_vkDebugReportCallbackEXT func);

	private:

		bool EnableValidation;

		bool quit;

		UniformBufferObject ubo{};
		Color color{};
		Window window{};
		float frames;
		std::chrono::time_point<std::chrono::steady_clock>
			lTime{};

		std::chrono::duration<float> durationSec{};

		uint32_t frame = 0;
		Time time{};
		DeviceAllocator allocator;

		VkInstance instance = VK_NULL_HANDLE;

		ContentManager manager;
		Shader shader;

		FRSWindowState state{};
		int windowHeight, windowWidth = 0;

		Controller controller{};
		Texture tex{};

		Device device;
		static FRSDebug_CallbackFunc callbackFunc;
		Reporter reporter = VK_NULL_HANDLE;
		Swapchain swapChain;

		GraphicPipeline graphPipeline{};
		Commander commander{};

		uint32_t oldHeight, oldWidth;

		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallbackFunc(
			VkDebugReportFlagsEXT flags,
			VkDebugReportObjectTypeEXT objType,
			uint64_t obj,
			size_t location,
			int32_t code,
			const char* layerPrefix,
			const char* msg,
			void* userData) {

			std::cout<<"Vulkan: "<<msg<<std::endl;
			return VK_FALSE;
		}

	};

}



