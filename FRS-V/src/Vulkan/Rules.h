#pragma once

#ifdef _WIN32
	#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <vulkan.h>

typedef unsigned char FRSchar;
typedef unsigned short FRSshort;
typedef unsigned int FRSint;
typedef unsigned long FRSlong;

typedef VkDebugReportCallbackEXT Reporter;

enum FRSMouse {
	LEFT = 0,
	RIGHT = 1,
	MIDDLE = 2
};

enum FRSWindowState {
	WINDOWED = 0,
	WINDOWEDFULL = 1,
	FULLSCREEN = 2,
};

enum FRSKeyState {
	PRESSED = 0,
	RELEASED = 0
};

enum FRS_STATE {
	SUCCESS = 1,
	FAILURE = 0
};