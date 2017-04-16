#include "Window.h"

namespace FRS {

	Window::Window(VkInstance instance,
		std::string title, FRSshort widthz,
		FRSshort heightz, FRSshort originX, FRSshort originY,
		FRSWindowState windowState) {

		this->instance = instance;

		WNDCLASSEX classEx;
		const char* ctitle = title.c_str();
		LPCWSTR str = LPCWSTR(ctitle);

		classEx.cbSize = sizeof(WNDCLASSEX);
		classEx.style = CS_HREDRAW | CS_VREDRAW;
		classEx.hIcon = LoadIcon(GetModuleHandleW(NULL), IDI_WINLOGO);
		classEx.hCursor = LoadCursor(GetModuleHandleW(NULL), IDC_ARROW);
		classEx.hInstance = GetModuleHandleW(NULL);
		classEx.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		classEx.lpszMenuName = NULL;
		classEx.lpszClassName = L"OPENGL";
		classEx.hIconSm = NULL;
		classEx.cbWndExtra = 0;
		classEx.cbClsExtra = 0;
		classEx.lpfnWndProc = (WNDPROC)SWndProc;

		if (!RegisterClassExW(&classEx)) {
			MessageBox(NULL, L"Failed to register Window!", L"Lol", MB_OK | MB_ICONEXCLAMATION);
			throw std::runtime_error("Can't create Window!");
		}

		long wndStyle = WS_OVERLAPPEDWINDOW;
		long dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;

		FRSshort posX = originX; FRSshort posY = originY;
		FRSshort widthN = widthz; FRSshort heightN = heightz;

		if (widthN == 0 && heightN == 0 && windowState == WINDOWED) {
			throw std::runtime_error("Garbage Window");
		}

		switch (windowState) {
		case FULLSCREEN:
			wndStyle = WS_POPUP;
			dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
			widthN = GetSystemMetrics(SM_CXSCREEN);
			heightN = GetSystemMetrics(SM_CYSCREEN);

			posX = posY = 0;

			DEVMODE fullMode;
			ZeroMemory(&fullMode, sizeof(DEVMODE));
			fullMode.dmSize = sizeof(DEVMODE);
			fullMode.dmPelsWidth = widthN;
			fullMode.dmPelsHeight = heightN;
			fullMode.dmBitsPerPel = 16;
			fullMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

			if (!ChangeDisplaySettings(&fullMode, CDS_FULLSCREEN))
				throw std::runtime_error("Failed to change settings to fullscreen");

			break;

		case WINDOWEDFULL:
			widthN = GetSystemMetrics(SM_CXSCREEN);
			heightN = GetSystemMetrics(SM_CYSCREEN);

			posX = posY = 0;

			break;

		default:
			break;
		}

		this->width = widthN;
		this->height = heightN;
		this->originX = posX;
		this->originY = posY;
		this->title = ctitle;

		this->mainWindow = CreateWindowExW(NULL,
			L"OPENGL", L"OPENGL",
			wndStyle | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
			posX, posY, widthN, heightN, NULL,
			NULL, GetModuleHandleW(NULL), nullptr);

		if (mainWindow == NULL) {
			throw std::runtime_error("cant create window");
		}

		ShowWindow(mainWindow, SW_SHOW);

		SetForegroundWindow(mainWindow);
		SetFocus(mainWindow);

		std::cout << "Window name: " << title << std::endl;
		std::cout << "\t\t" << "Width: " << widthN << "; Height: " << heightN << std::endl;
		std::cout << "\t\t" << "OriginX: " << originX << "; OriginY: " << originY << std::endl;

		VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};

		surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surfaceCreateInfo.flags = 0;
		surfaceCreateInfo.hinstance = GetModuleHandle(NULL);
		surfaceCreateInfo.hwnd = mainWindow;

		surfaceCreateInfo.pNext = NULL;

		auto CreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR");

		VkResult result = CreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &(surface));

		FRS_ASSERT(result != VK_SUCCESS, "Failed create window surface!");

		if (SetPropW(mainWindow, L"FRSEngine", (HANDLE)this)) {
			std::cout << "Set Prop success" << std::endl;
		}

		ShouldQuit = false;
}

	/*Window::~Window() {

	}*/

	void DestroyWindow(Window window) {
		vkDestroySurfaceKHR((window.instance), window.surface, nullptr);
		ChangeDisplaySettings(NULL, 0);
	}

	FRSshort GetScreenWidth() {
		return GetSystemMetrics(SM_CXSCREEN);
	}

	FRSshort GetScreenHeight() {
		return GetSystemMetrics(SM_CYSCREEN);
	}

	LRESULT Window::SWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

		Window* window = (Window*)GetPropW(hwnd, L"FRSEngine");

		if (!window) {
			return DefWindowProcW(hwnd, msg, wParam, lParam);
		}

		switch (msg) {

			case WM_CLOSE:
			{
				window->ShouldQuit = true;
				return 0;
			}

			case WM_KEYDOWN:
			{
				if (window->eventStruct.keyFuncCallBack) {
					window->eventStruct.keyFuncCallBack(wParam, PRESSED);
				}

				break;
			}

			case WM_SIZE:
			{
				if (window->eventStruct.resizeFunc) {
					RECT windowRect;
					GetWindowRect(hwnd, &windowRect);
					window->width = windowRect.right - windowRect.left, windowRect.bottom;
					window->height = windowRect.bottom - windowRect.top;
					window->eventStruct.resizeFunc(windowRect.right - windowRect.left, windowRect.bottom- windowRect.top);
				}

				break;
			}

		}

		return DefWindowProcW(hwnd, msg, wParam, lParam);
	}

	void CreateVulkanWindow(Window* window, VkInstance instance, std::string title, int width, int height, int originX, int originY,
		FRSWindowState state) {

		*window = Window(instance, title, width, height, originX, originY, state);

		SetPropW(window->GetWin32Window(), L"FRSEngine", window);

	}

	//Choose pixel format
	//Set pixel format
	//CreateContext
	//Make context
	void FRSPollEvents() {

		MSG msg;

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {

			TranslateMessage(&msg);
			DispatchMessage(&msg);

		}

	}

}