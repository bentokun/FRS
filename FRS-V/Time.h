#pragma once

#include <chrono>
#include <thread>

#ifdef _WIN32
#ifdef FRSV_EXPORTS
#define TFAPI __declspec(dllexport)
#else
#define TFAPI __declspec(dllimport)
#endif
#endif

namespace FRS {

	struct TFAPI Time {
		uint32_t frame = 0;
		std::chrono::time_point<std::chrono::steady_clock> lTime{};

		float GetFrameRate();
	};


	TFAPI void AdjustCurrentThreadFPS(int fps);

}