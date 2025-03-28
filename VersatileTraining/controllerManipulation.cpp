#include "pch.h"
#include "versatileTraining.h"

//void VersatileTraining::checkForR1Press() {
//	if (controllers.empty() || !controllers[0]) {
//		LOG("No controller connected.");
//		return;
//	}
//
//	DIJOYSTATE2 js;
//
//	HRESULT hr = controllers[0]->Poll();
//	if (FAILED(hr)) {
//		LOG("Controller polling failed. Attempting to reacquire...");
//		controllers[0]->Acquire();
//		return;
//	}
//
//	hr = controllers[0]->GetDeviceState(sizeof(DIJOYSTATE2), &js);
//	if (FAILED(hr)) {
//		LOG("Failed to get device state.");
//		return;
//	}
//
//	// Check if R1 (Button 5) is pressed
//	if (js.rgbButtons[5] & 0x80 && !R1AlreadyPressed) {
//		LOG("R1 button pressed");
//		tempBoostAmount++;
//		R1AlreadyPressed = true;
//	}
//	else if (js.rgbButtons[5] & 0x80 && R1AlreadyPressed) {
//		auto now = std::chrono::steady_clock::now();
//		std::chrono::duration<double> elapsed = now - lastUpdateTime;
//		if (elapsed.count() > 0.5) {
//			LOG("R1 button held for 0.5seconds, incrementing tempBoost");
//			tempBoostAmount++;
//			lastUpdateTime = now;
//		}
//	}
//	else if (!(js.rgbButtons[5] & 0x80) && R1AlreadyPressed) {
//		LOG("R1 button released");
//		R1AlreadyPressed = false;
//	}
//}



BOOL CALLBACK VersatileTraining::EnumDevicesCallback(const DIDEVICEINSTANCE* instance, VOID* context)
{
	VersatileTraining* pThis = static_cast<VersatileTraining*>(context);

	std::wstring wstr(instance->tszProductName);
	std::string str(wstr.begin(), wstr.end());

	LOG("Found device: {}", str);

	LPDIRECTINPUTDEVICE8 controller;
	HRESULT hr = pThis->dinput->CreateDevice(instance->guidInstance, &controller, NULL);
	if (FAILED(hr)) {
		LOG("Failed to create device for: {}", str);
		return DIENUM_CONTINUE;
	}

	hr = controller->SetDataFormat(&c_dfDIJoystick2);
	if (FAILED(hr)) {
		LOG("Failed to set data format for device: {}", str);
		return DIENUM_CONTINUE;
	}


	hr = controller->SetCooperativeLevel(GetActiveWindow(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(hr)) {
		LOG("Failed to set cooperative level for device: {}", str);
		return DIENUM_CONTINUE;
	}


	hr = controller->Acquire();
	if (FAILED(hr)) {
		LOG("Failed to acquire device: {}", str);
		return DIENUM_CONTINUE;
	}

	LOG("Successfully acquired device: {}", str);
	pThis->controllers.push_back(controller);

	return DIENUM_CONTINUE;
}
void VersatileTraining::checkForButtonPress(int buttonIndex) {
	if (controllers.empty() || !controllers[0]) {
		//LOG("No controller connected.");
		enumerateControllers();
		if (controllers.empty() || !controllers[0]) {
			LOG("No controller connected.");
			return;
		}
		
		checkForButtonPress(buttonIndex);
	
	}

	if (!buttonCallbacks.count(buttonIndex)) {
		LOG("No callback registered for button {}", buttonIndex);
		return;
	}

	DIJOYSTATE2 js;

	HRESULT hr = controllers[0]->Poll();
	if (FAILED(hr)) {
		LOG("Controller polling failed. Attempting to reacquire...");
		controllers[0]->Acquire();
		return;
	}

	hr = controllers[0]->GetDeviceState(sizeof(DIJOYSTATE2), &js);
	if (FAILED(hr)) {
		LOG("Failed to get device state.");
		return;
	}

	if (buttonIndex < 0 || buttonIndex >= 128) {
		LOG("Invalid button index: {}", buttonIndex);
		return;
	}

	bool isButtonPressed = (js.rgbButtons[buttonIndex] & 0x80);
	auto& state = buttonStates[buttonIndex];

	if (isButtonPressed && !state.isPressed) {
		LOG("Button {} pressed", buttonIndex);
		state.isPressed = true;
		state.lastUpdateTime = std::chrono::steady_clock::now();

		
		buttonCallbacks[buttonIndex]();
	}
	else if (isButtonPressed && state.isPressed) {
		auto now = std::chrono::steady_clock::now();
		std::chrono::duration<double> elapsed = now - state.lastUpdateTime;
		if (elapsed.count() > 0.01) {
			LOG("Button {} held for 0.5 seconds", buttonIndex);
			buttonCallbacks[buttonIndex]();
			
			state.lastUpdateTime = now;
		}
	}
	else if (!isButtonPressed && state.isPressed) {
		LOG("Button {} released", buttonIndex);
		state.isPressed = false;
	}
}



void VersatileTraining::initializeCallBacks() {


	/*registerButtonCallback(5, [this]() { IncrementTempBoost(); });
	registerButtonCallback(6, [this]() { IncrementTempStartingVelocity(); });*/

	registerButtonCallback(4, [this]() { rollLeft(); });
	registerButtonCallback(5, [this]() { rollRight(); });
}

void VersatileTraining::IncrementTempBoost() {
	tempBoostAmount++;
	
	LOG("Incrementing tempBoostAmount to {}", tempBoostAmount);
}

void VersatileTraining::rollLeft() {
	rotationToApply.Roll -= 500;
	LOG("Rolling left");
}
void VersatileTraining::rollRight() {
	rotationToApply.Roll += 500;
	LOG("Rolling right");
}
void VersatileTraining::IncrementTempStartingVelocity() {
	tempStartingVelocity++;
	LOG("Incrementing tempStartingVelocity to {}", tempStartingVelocity);
}

void VersatileTraining::registerButtonCallback(int buttonIndex, ButtonCallback callback) {
	buttonCallbacks[buttonIndex] = callback;
	buttonStates[buttonIndex] = { false, std::chrono::steady_clock::now() };
}

void VersatileTraining::enumerateControllers() {
	//LPDIRECTINPUT8 dinput;
	DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&dinput, NULL);


	auto callback = [](const DIDEVICEINSTANCE* instance, VOID* context) -> BOOL {
		
		VersatileTraining* pThis = static_cast<VersatileTraining*>(context);
		return pThis->EnumDevicesCallback(instance, context);
		};

	dinput->EnumDevices(DI8DEVCLASS_GAMECTRL, callback, this, DIEDFL_ATTACHEDONLY);

}