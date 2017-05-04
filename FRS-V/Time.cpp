#include "Time.h"
#include <iostream>

namespace FRS {
	float Time::GetFrameRate() {

		frame++;

		auto thisTime = std::chrono::high_resolution_clock::now();
		auto durationSec = std::chrono::duration_cast
			<std::chrono::duration<float>>(thisTime - lTime);

		if (durationSec.count() > 0.25f) {
			float fps = frame / durationSec.count();

			previousFramerate = fps;

			frame = 0;
			lTime = thisTime;

			return fps;
		}

		return previousFramerate;

	}

	void AdjustCurrentThreadFPS(int fps) {

		auto lastTime = std::chrono::system_clock::now();
		auto time = std::chrono::system_clock::now();
		std::chrono::duration
			<double, std::milli> duration = (time - lastTime);

		std::chrono::duration<double,
			std::milli> durationPerFrame(1/ fps * CLOCKS_PER_SEC);

		if (duration.count() < durationPerFrame.count()) {
			std::chrono::duration<double, std::milli> delta_ms(durationPerFrame.count() - duration.count());
			auto ms_duration = std::chrono::duration_cast<std::chrono::milliseconds>(delta_ms);
			std::this_thread::sleep_for(std::chrono::milliseconds(ms_duration.count()));
		}

		auto b = std::chrono::system_clock::now();
		std::chrono::duration<double, std::milli> sleep_time = b - time;

	}

}