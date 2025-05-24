#include "pch.h"
#include "ControllerManager.h"
#include "src/core/VersatileTraining.h"



void ControllerManager::cleanup(){
	for (auto& controller : controllers) {
		if (controller) {
			controller->Unacquire(); // Unacquire before release
			controller->Release();   // Release the DirectInput device
			controller = nullptr;
		}
	}
	controllers.clear(); // Clear the vector
	 

	if (dinput) {
		dinput->Release();
		dinput = nullptr;
		 
	}
}
void ControllerManager::initializeCallBacks() {
	/*registerButtonCallback(5, [this]() { IncrementTempBoost(); });
	registerButtonCallback(6, [this]() { IncrementTempStartingVelocity(); });*/
	 
	registerButtonCallback(4, [this]() { rollLeft(); });
	registerButtonCallback(5, [this]() { rollRight(); });
}

void ControllerManager::registerButtonCallback(int buttonIndex, ButtonCallback callback) {
	 
	buttonCallbacks[buttonIndex] = callback;
	buttonStates[buttonIndex] = { false, std::chrono::steady_clock::now() };
}

void ControllerManager::rollLeft() {
	
	 
}
void ControllerManager::rollRight() {
	
	 
}
void ControllerManager::checkForButtonPress(int buttonIndex){
	if (controllers.empty() || !controllers[0]) {
		// 
		enumerateControllers();
		if (controllers.empty() || !controllers[0]) {
			 
			return;
		}

		checkForButtonPress(buttonIndex);

	}

	if (!buttonCallbacks.count(buttonIndex)) {
		 
		return;
	}

	DIJOYSTATE2 js;

	HRESULT hr = controllers[0]->Poll();
	if (FAILED(hr)) {
		 
		controllers[0]->Acquire();
		return;
	}

	hr = controllers[0]->GetDeviceState(sizeof(DIJOYSTATE2), &js);
	if (FAILED(hr)) {
		 
		return;
	}

	if (buttonIndex < 0 || buttonIndex >= 128) {
		 
		return;
	}

	bool isButtonPressed = (js.rgbButtons[buttonIndex] & 0x80);
	auto& state = buttonStates[buttonIndex];

	if (isButtonPressed && !state.isPressed) {
		 
		state.isPressed = true;
		state.lastUpdateTime = std::chrono::steady_clock::now();


		buttonCallbacks[buttonIndex]();
	}
	else if (isButtonPressed && state.isPressed) {
		auto now = std::chrono::steady_clock::now();
		std::chrono::duration<double> elapsed = now - state.lastUpdateTime;
		if (elapsed.count() > 0.01) {
			 
			buttonCallbacks[buttonIndex]();

			state.lastUpdateTime = now;
		}
	}
	else if (!isButtonPressed && state.isPressed) {
		 
		state.isPressed = false;
	}
}
void ControllerManager::enumerateControllers(){
	DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&dinput, NULL);


	auto callback = [](const DIDEVICEINSTANCE* instance, VOID* context) -> BOOL {

		ControllerManager* pThis = static_cast<ControllerManager*>(context);
		return pThis->EnumDevicesCallback(instance, context);
		};

	dinput->EnumDevices(DI8DEVCLASS_GAMECTRL, callback, this, DIEDFL_ATTACHEDONLY);
}

 BOOL CALLBACK ControllerManager::EnumDevicesCallback(const DIDEVICEINSTANCE* instance, VOID* context){
	 ControllerManager* pThis = static_cast<ControllerManager*>(context);

	 std::wstring wstr(instance->tszProductName);
	 std::string str(wstr.begin(), wstr.end());

	  

	 LPDIRECTINPUTDEVICE8 controller;
	 HRESULT hr = pThis->dinput->CreateDevice(instance->guidInstance, &controller, NULL);
	 if (FAILED(hr)) {
		  
		 return DIENUM_CONTINUE;
	 }

	 hr = controller->SetDataFormat(&c_dfDIJoystick2);
	 if (FAILED(hr)) {
		  
		 return DIENUM_CONTINUE;
	 }


	 hr = controller->SetCooperativeLevel(GetActiveWindow(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	 if (FAILED(hr)) {
		  
		 return DIENUM_CONTINUE;
	 }


	 hr = controller->Acquire();
	 if (FAILED(hr)) {
		  
		 return DIENUM_CONTINUE;
	 }

	  
	 pThis->controllers.push_back(controller);

	 return DIENUM_CONTINUE;
 }