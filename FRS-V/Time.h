#pragma once

#pragma warning (disable: 4251 4267)

#include <chrono>
#include <thread>

#ifdef _WIN32
#ifdef FRSV_EXPORTS
#define TFSAPI __declspec(dllexport)
#else
#define TFSAPI __declspec(dllimport)
#endif
#endif

namespace FRS {

	struct TFSAPI Time {
		uint32_t frame = 0;
		std::chrono::time_point<std::chrono::steady_clock> lTime;

		float previousFramerate = 0;

		float GetFrameRate();
	};


	TFSAPI void AdjustCurrentThreadFPS(int fps);

}