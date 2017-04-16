#pragma once

#include <Windows.h>
#include <string>
#include <memory>
#include <functional>
#include <vector>

#include <iostream>

#include "Rules.h"
#include "Assert.h"

#ifdef _WIN32
#ifdef FRSV_EXPORTS
#define TFAPI __declspec(dllexport)
#else
#define TFAPI __declspec(dllimport)
#endif
#endif

template class TFAPI std::function<void(int, FRSKeyState)>;
template class TFAPI std::function<void(int, int)>;
typedef std::function<void(int, FRSKeyState)> FRSKEYBOARDFUNC;
typedef std::function<void(int, int)> FRSRESIZEFUNC;

namespace FRS {

#ifdef WIN32

	struct TFAPI Window {
	public:

		std::shared_ptr<Window> sharedWindow;

		Window(VkInstance instance, std::string title, FRSshort widthz = 0,
			FRSshort heightz = 0, FRSshort originX = 0, FRSshort originY = 0,
			FRSWindowState windowState = WINDOWED);
		
		Window() {};

		//~Window();

		bool ShouldQuit = false;


		HWND& GetWin32Window() {
			return mainWindow;
		}

		VkSurfaceKHR surface{ VK_NULL_HANDLE };

		friend void DestroyWindow(Window window);

		void SetKeyboardCallback(FRSKEYBOARDFUNC func) {
			eventStruct.keyFuncCallBack = func;
		}

		void SetResizeCallback(std::function<void(int, int)> func) {
			eventStruct.resizeFunc = func;
		}

		VkExtent2D GetWindowExtent() {
			VkExtent2D extent = {};
			extent.width = width;
			extent.height = height;

			return extent;
		}

	private:

		HWND mainWindow;

		VkInstance instance;

		std::string title;
		FRSshort width, height, originX, originY;
		HICON winIcon; HCURSOR winCursor;

		struct eventFunc{
			FRSKEYBOARDFUNC keyFuncCallBack{ nullptr };
			FRSRESIZEFUNC resizeFunc{ nullptr };
		} eventStruct;

		static LRESULT CALLBACK SWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	};

#endif

	TFAPI FRSshort GetScreenWidth();
	TFAPI FRSshort GetScreenHeight();

	TFAPI void CreateVulkanWindow(Window* window,VkInstance instance,std::string title, int width, int height,int originX = 0 , int originY = 0,
		FRSWindowState state = WINDOWED);

	//Will make the window havena
	TFAPI void FRSPollEvents();


}
