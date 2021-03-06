#pragma once

#ifdef _WIN32
	#define VK_USE_PLATFORM_WIN32_KHR

	#ifdef FRSV_EXPORTS
	#define TFSAPI __declspec(dllexport)
	#else
	#define TFSAPI __declspec(dllimport)
	#endif

#endif

#include <string>

#include <Windows.h>
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

enum Stage {
	VERTEX_SHADER_STAGE = VK_SHADER_STAGE_VERTEX_BIT,
	FRAGMENT_SHADER_STAGE = VK_SHADER_STAGE_FRAGMENT_BIT,
	COMPUTE_SHADER_STAGE = VK_SHADER_STAGE_COMPUTE_BIT,
	GEOMETRY_SHADER_STAGE = VK_SHADER_STAGE_GEOMETRY_BIT,
	ALL_SHADER_STAGE = VK_SHADER_STAGE_ALL_GRAPHICS,
	TESSELLATION_EVALUATION_STAGE = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
	TESSELLATION_CONTROL_STAGE = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
	UNDENTIFIED
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

TFSAPI std::wstring ConvertToWString(std::string str);




