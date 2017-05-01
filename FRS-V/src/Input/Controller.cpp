#include "Controller.h"
#include <iostream>

namespace FRS {

	int numberController = -1;

	Controller::Controller() {
		numberController++;
		buttonA = buttonB = buttonX = buttonY = back = start = false;
		stickLX = stickLY = stickRX = stickRY = 0;
		DPad.Up = DPad.Down = DPad.Left = DPad.Right = false;
		leftTrigger = rightTrigger = 0;
		batteryLevel = batteryType = 0;
	}

	bool Controller::GetState() {

		#ifdef _WIN32

			ZeroMemory(&state, sizeof(XINPUT_STATE));

			if (XInputGetState(numberController, &state) == ERROR_SUCCESS) {

				buttonA = ((state.Gamepad.wButtons & XINPUT_GAMEPAD_A) != 0);
				buttonB = ((state.Gamepad.wButtons & XINPUT_GAMEPAD_B) != 0);
				buttonX = ((state.Gamepad.wButtons & XINPUT_GAMEPAD_X) != 0);
				buttonY = ((state.Gamepad.wButtons & XINPUT_GAMEPAD_Y) != 0);
				
				back    = ((state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) != 0);
				start   = ((state.Gamepad.wButtons & XINPUT_GAMEPAD_START) != 0);

				DPad.Up    = ((state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0);
				DPad.Down  = ((state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0);
				DPad.Right = ((state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0);
				DPad.Left  = ((state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) != 0);

				leftTrigger = (static_cast<float>(state.Gamepad.bLeftTrigger) / 255);
				rightTrigger = (static_cast<float>(state.Gamepad.bRightTrigger) / 255);

				const float deadzoneX = 0.05f;
				const float deadzoneY = 0.02f;

				float normLX = FRSML::Max(-1, static_cast<float>(state.Gamepad.sThumbLX) / 32767);
				float normLY = FRSML::Max(-1, static_cast<float>(state.Gamepad.sThumbLY) / 32767);

				float normRX = FRSML::Max(-1, static_cast<float>(state.Gamepad.sThumbRX) / 32767);
				float normRY = FRSML::Max(-1, static_cast<float>(state.Gamepad.sThumbRY) / 32767);

				stickLX = static_cast<float>(FRSML::Abs(normLX) < deadzoneX ? 0 : (FRSML::Abs(normLX) - deadzoneX)*(FRSML::Sign(normLX)));
				stickLY = static_cast<float>(FRSML::Abs(normLY) < deadzoneY ? 0 : (FRSML::Abs(normLY) - deadzoneY)*(FRSML::Sign(normLY)));

				stickRX = static_cast<float>(FRSML::Abs(normRX) < deadzoneX ? 0 : (FRSML::Abs(normRX) - deadzoneX)*(FRSML::Sign(normRX)));
				stickRY = static_cast<float>(FRSML::Abs(normRY) < deadzoneY ? 0 : (FRSML::Abs(normRY) - deadzoneY)*(FRSML::Sign(normRY)));

				return true;
			}

			return false;

		#endif
	}

	bool Controller::GetBatteryInformation() {

		#ifdef _WIN32
				ZeroMemory(&batteryInformation, sizeof(XINPUT_BATTERY_INFORMATION));

				if (XInputGetBatteryInformation(numberController, XINPUT_DEVTYPE_GAMEPAD,
					&batteryInformation)) {

					batteryLevel = batteryInformation.BatteryLevel;
					batteryType = batteryInformation.BatteryType;

					return true;
				}
		
				return false;
		#endif

	}

	bool Controller::Vibrate(int leftMotor, int rightMotor) {
		#ifdef _WIN32
				XINPUT_VIBRATION vibration;

				vibration.wLeftMotorSpeed  = leftMotor;
				vibration.wRightMotorSpeed = rightMotor;
	

				if (XInputSetState(numberController, &vibration)) {
					return true;
				}

				return false;
		#endif
	}

	void GetAvailableController(bool port[20]) {
			#ifdef _WIN32
					for (DWORD i = 0; i < XUSER_MAX_COUNT; i++) {
						XINPUT_STATE state;
						ZeroMemory(&state, sizeof(XINPUT_STATE));

						if (XInputGetState(i, &state)) {
							port[i] = true;
						}
						else port[i] = false;
					}
			#endif
	}

}

