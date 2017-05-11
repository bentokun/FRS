#pragma once

#pragma warning (disable: 4251 4267)

#include <string>
#include <memory>
#include <functional>
#include <vector>

#include <iostream>

#include <Windows.h>

#include "Rules.h"
#include "Assert.h"

#ifdef _WIN32
	#ifdef FRSV_EXPORTS
	#define TFSAPI __declspec(dllexport)
	#else
	#define TFSAPI __declspec(dllimport)
	#endif
#endif

template class TFSAPI std::function<void(int, FRSKeyState)>;
template class TFSAPI std::function<void(int, int)>;
typedef std::function<void(int, FRSKeyState)> KeyboardFunc;
typedef std::function<void(int, int)> ResizeFunc;

namespace FRS {

#ifdef WIN32

	struct TFSAPI Window {
	public:

		std::shared_ptr<Window> sharedWindow;

		Window(VkInstance instance, std::string title, FRSshort widthz = 0,
			FRSshort heightz = 0, FRSshort originX = 0, FRSshort originY = 0,
			FRSWindowState windowState = WINDOWED);
		
		Window() {};

		HWND& GetWin32Window() {
			return mainWindow;
		}

		VkSurfaceKHR surface{ VK_NULL_HANDLE };

		friend void DestroyWindow(Window window);

		void SetKeyboardCallback(KeyboardFunc func) {
			eventStruct.keyFuncCallBack = func;
		}

		void SetResizeCallback(std::function<void(int, int)> func) {
			eventStruct.resizeFunc = func;
		}

#pragma region GETTER
		VkExtent2D GetWindowExtent() {

			VkExtent2D extent = {};
			extent.width = width;
			extent.height = height;

			return extent;
		}

		void LoadWindowIcon(LPWSTR path, int width, int height) {
			winIcon = static_cast<HICON>(LoadImageW(NULL, path,
				IMAGE_ICON, width, height, LR_LOADFROMFILE));
		}
#pragma endregion
		bool ShouldQuit = false;

	private:

		HWND mainWindow;

		VkInstance instance;

		std::string title;
		FRSshort width, height, originX, originY;
		HICON winIcon; HCURSOR winCursor;

		struct eventFunc{
			KeyboardFunc keyFuncCallBack{ nullptr };
			ResizeFunc resizeFunc{ nullptr };
		} eventStruct;

		static LRESULT CALLBACK SWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	};

#endif

	TFSAPI FRSshort GetScreenWidth();
	TFSAPI FRSshort GetScreenHeight();

	TFSAPI void CreateVulkanWindow(Window* window,VkInstance instance,std::string title, int width, int height,int originX = 0 , int originY = 0,
		FRSWindowState state = WINDOWED);

	//Will make the window havena
	TFSAPI void PollEvents();


}
