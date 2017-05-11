#pragma once

#ifdef _WIN32

#include <Windows.h>
#include <Xinput.h>

#include <FRSML>

#ifdef FRSV_EXPORTS
		#define TFSAPI __declspec(dllexport)
	#else
		#define TFSAPI __declspec(dllimport)
	#endif
#else

	#define TFSAPI

#endif


namespace FRS {

	struct TFSAPI Controller {
	public:

		Controller();

		bool GetState();
		bool GetBatteryInformation();
		bool Vibrate(int leftMotor, int rightMotor);

		bool buttonA, buttonB, buttonX, buttonY, back, start = false;

		struct Pad {
			bool Up, Down, Left, Right = false;
		} DPad;

		float stickLX, stickLY, stickRX, stickRY = 0;
		float leftTrigger, rightTrigger = 0;
		float batteryLevel, batteryType = 0;

	private:

		XINPUT_STATE state;
		XINPUT_BATTERY_INFORMATION batteryInformation;

	};

	void TFSAPI GetAvailableController(bool port[20]);

}