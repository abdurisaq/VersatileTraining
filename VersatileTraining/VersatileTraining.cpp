#include "pch.h"
#include "VersatileTraining.h"


BAKKESMOD_PLUGIN(VersatileTraining, "write a plugin description here", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void VersatileTraining::onLoad()
{
	_globalCvarManager = cvarManager;
	LPDIRECTINPUT8 directInput = nullptr;
	LPDIRECTINPUTDEVICE8 controller = nullptr;
	
	LOG("Plugin loaded!!");
	
	this->loadHooks();
	
	gameWrapper->RegisterDrawable([this](CanvasWrapper canvas) {
		Render(canvas);
	});
	
	cvarManager->registerNotifier(
		"findController",
		[this](std::vector<std::string> args) {
			HWND hwnd = FindWindowA("LaunchUnrealUWindowsClient", "Rocket League (64-bit, DX11, Cooked)");
			if (!hwnd) {
				LOG("Failed to get Rocket League window handle.");
				return;
			}
			else {
				LOG("Got Rocket League window handle.");
			}

			enumerateControllers();
		}, "check to see if there's a connected controller", PERMISSION_ALL
	);
	/*gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.GameEvent_TrainingEditor_TA.LoadRound", [this](ActorWrapper cw, void* params, std::string eventName) {
		VersatileTraining::getTrainingData(cw, params, eventName);
		});*/

	/*gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.GameEvent_TrainingEditor_TA.LoadRound", [this](ActorWrapper cw, void* params, std::string eventName) {
		auto tw = ((TrainingEditorWrapper)cw.memory_address);
		GameEditorSaveDataWrapper data = tw.GetTrainingData();
		TrainingEditorSaveDataWrapper td = data.GetTrainingData();
		LOG("Training data found: {}", td.GetCode().ToString());
		});*/
	// !! Enable debug logging by setting DEBUG_LOG = true in logging.h !!
	//DEBUGLOG("VersatileTraining debug mode enabled");

	// LOG and DEBUGLOG use fmt format strings https://fmt.dev/latest/index.html
	//DEBUGLOG("1 = {}, 2 = {}, pi = {}, false != {}", "one", 2, 3.14, true);

	//cvarManager->registerNotifier("my_aweseome_notifier", [&](std::vector<std::string> args) {
	//	LOG("Hello notifier!");
	//}, "", 0);

	//auto cvar = cvarManager->registerCvar("template_cvar", "hello-cvar", "just a example of a cvar");
	//auto cvar2 = cvarManager->registerCvar("template_cvar2", "0", "just a example of a cvar with more settings", true, true, -10, true, 10 );

	//cvar.addOnValueChanged([this](std::string cvarName, CVarWrapper newCvar) {
	//	LOG("the cvar with name: {} changed", cvarName);
	//	LOG("the new value is: {}", newCvar.getStringValue());
	//});

	//cvar2.addOnValueChanged(std::bind(&VersatileTraining::YourPluginMethod, this, _1, _2));

	// enabled decleared in the header
	//enabled = std::make_shared<bool>(false);
	//cvarManager->registerCvar("TEMPLATE_Enabled", "0", "Enable the TEMPLATE plugin", true, true, 0, true, 1).bindTo(enabled);

	//cvarManager->registerNotifier("NOTIFIER", [this](std::vector<std::string> params){FUNCTION();}, "DESCRIPTION", PERMISSION_ALL);
	//cvarManager->registerCvar("CVAR", "DEFAULTVALUE", "DESCRIPTION", true, true, MINVAL, true, MAXVAL);//.bindTo(CVARVARIABLE);
	//gameWrapper->HookEvent("FUNCTIONNAME", std::bind(&TEMPLATE::FUNCTION, this));
	//gameWrapper->HookEventWithCallerPost<ActorWrapper>("FUNCTIONNAME", std::bind(&VersatileTraining::FUNCTION, this, _1, _2, _3));
	//gameWrapper->RegisterDrawable(bind(&TEMPLATE::Render, this, std::placeholders::_1));


	//gameWrapper->HookEvent("Function TAGame.Ball_TA.Explode", [this](std::string eventName) {
	//	LOG("Your hook got called and the ball went POOF");
	//});
	// You could also use std::bind here
	//gameWrapper->HookEvent("Function TAGame.Ball_TA.Explode", std::bind(&VersatileTraining::YourPluginMethod, this);
}

void VersatileTraining::loadHooks() {

	/*gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.GameEvent_TrainingEditor_TA.LoadRound", [this](ActorWrapper cw, void* params, std::string eventName) {
		VersatileTraining::getTrainingData(cw, params, eventName);
		});*/

	gameWrapper->HookEventWithCallerPost<GameEditorWrapper>("Function GameEvent_TrainingEditor_TA.ShotSelection.StartEditing", [this](GameEditorWrapper cw, void* params, std::string eventName) {
		LOG("new training pack opened and editing started--------------");
		});
	gameWrapper->HookEventWithCallerPost<TrainingEditorWrapper>("Function GameEvent_TrainingEditor_TA.EditorMode.StopEditing", [this](TrainingEditorWrapper cw, void* params, std::string eventName) {
		LOG("stopped editing");
		});

	gameWrapper->HookEventWithCallerPost<TrainingEditorWrapper>("Function TAGame.Ball_GameEditor_TA.EditingEnd", [this](TrainingEditorWrapper cw, void* params, std::string eventName) {
		LOG("car is being edited");
		editingVariances = true;
		});
	gameWrapper->HookEventWithCallerPost<TrainingEditorWrapper>("Function TAGame.GameEditor_Actor_TA.EditingEnd", [this](TrainingEditorWrapper cw, void* params, std::string eventName) {
		LOG("ball is being edited");
		editingVariances = false;
		});
	//TAGame.PlayerController_TA.EventTrainingEditorActorModified
	gameWrapper->HookEvent("Function TAGame.PlayerController_TA.EventTrainingEditorActorModified",
		[this](std::string eventName) {
			if (editingVariances) {
				//LOG("Training editor actor modified");
				checkForR1Press();
			}
		}
	);

	

		
}

void VersatileTraining::checkForR1Press() {
	//DIJOYSTATE2 js;
	//if (FAILED(controller->Poll())) {
	//	// Reacquire device if necessary
	//	controller->Acquire();
	//	return;
	//}
	//if (FAILED(controller->GetDeviceState(sizeof(DIJOYSTATE2), &js))) {
	//	//std::cerr << "Failed to get device state" << std::endl;
	//	LOG("Failed to get device state");
	//	return;
	//}

	//// Check if R1 (Right Shoulder) is pressed
	//if (js.rgbButtons[5] & 0x80) { // R1 is usually mapped to button index 5 on PS controllers
	//	//std::cout << "R1 Button Pressed" << std::endl;
	//	LOG("R1 Button Pressed");
	//}
	//else {
	//	//std::cout << "R1 Button Not Pressed" << std::endl;
	//	LOG("R1 Button Not Pressed");
	//}
}

TrainingEditorWrapper VersatileTraining::GetTrainingEditor() {
	auto serv = gameWrapper->GetGameEventAsServer();
	if (!serv) return { 0 };
	return TrainingEditorWrapper(serv.memory_address);
}

void VersatileTraining::getTrainingData(ActorWrapper cw, void* params, std::string eventName) {

	/*TrainingEditorWrapper tew = GetTrainingEditor();
	if (!tew) return;

	auto current = tew.GetActiveRoundNumber();*/

	auto tw = ((TrainingEditorWrapper)cw.memory_address);
	GameEditorSaveDataWrapper data = tw.GetTrainingData();
	TrainingEditorSaveDataWrapper td = data.GetTrainingData();
	LOG("Training data found1: {}", td.GetCode().ToString());
	int totalRounds = tw.GetTotalRounds();
	LOG("Training num rounds: {}", totalRounds);

	int currentShot = tw.GetActiveRoundNumber();
	LOG("Training current shot : {}", currentShot);

	std::string code = td.GetCode().ToString();

	auto foundCode =  trainingData.find(code);
	if (foundCode != trainingData.end()) {
		LOG("Training data found: {}", code);
		foundCode->second.name = td.GetTM_Name().ToString();
		foundCode->second.numShots = totalRounds;
		LOG("loaded training name: {}", foundCode->second.name);
		cvarManager->executeCommand("sv_training_enabled 1");
		cvarManager->executeCommand("sv_training_limitboost " + std::to_string(foundCode->second.boostAmounts[0]));
		//sv_training_player_velocity (1700, 1900);
		cvarManager->executeCommand("sv_training_player_velocity ("+ std::to_string(foundCode->second.startingVelocityMin[0]) + "," + std::to_string(foundCode->second.startingVelocityMax[0]) + ")");

	}
	else {
		LOG("Training data not found: {}", code);
		cvarManager->executeCommand("sv_training_enabled 0");
		cvarManager->executeCommand("sv_training_limitboost -1");

	}
	/*if (code == "0624-600D-000E-F2A7") {
		cvarManager->executeCommand("sv_training_enabled 1");
		cvarManager->executeCommand("sv_training_limitboost 100");
	}
	else {
		cvarManager->executeCommand("sv_training_enabled 0");
		cvarManager->executeCommand("sv_training_limitboost -1");

	}*/

	//executecommand ("sv_training_enabled 1); to turn on custom training variance, and if not turn it off.
	
}

void VersatileTraining::setTrainingVariables(ActorWrapper cw, void* params, std::string eventName) {

}

void VersatileTraining::restartTraining() {
	LOG("Restarting training");
	ServerWrapper server = gameWrapper->GetGameEventAsServer();

	TrainingEditorWrapper tew(server.memory_address);

	

}

void VersatileTraining::onUnload() {
	LOG("Unloading Versatile Training");
	CleanUp();
}

void VersatileTraining::CleanUp() {
	for (auto& controller : controllers) {
		if (controller) {
			controller->Unacquire(); // Unacquire before release
			controller->Release();   // Release the DirectInput device
			controller = nullptr;
		}
	}
	controllers.clear(); // Clear the vector
	LOG("All controllers cleaned up.");

	if (dinput) {
		dinput->Release();
		dinput = nullptr;
		LOG("DirectInput object released.");
	}
}

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

void VersatileTraining::enumerateControllers() {
	//LPDIRECTINPUT8 dinput;
	DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&dinput, NULL);

	
	auto callback = [](const DIDEVICEINSTANCE* instance, VOID* context) -> BOOL {
		//  need to cast context to the appropriate type
		VersatileTraining* pThis = static_cast<VersatileTraining*>(context);
		return pThis->EnumDevicesCallback(instance, context);
		};

	dinput->EnumDevices(DI8DEVCLASS_GAMECTRL, callback, this, DIEDFL_ATTACHEDONLY);

}

//std::vector<std::pair<int, int>> runLengthEncode(std::vector<int> arr) {
//
//	std::vector<std::pair<int, int>> result;
//	int count = 1;
//	int pastInt = arr[0];
//	for (int i = 1; i < arr.size(); i++) {
//		if (arr[i] == pastInt) {
//			count++;
//		}
//		else {
//			result.push_back(std::make_pair(pastInt, count));
//			count = 1;
//			pastInt = arr[i];
//		}
//	}
//
//}

//
//std::string VersatileTraining::encodeTrainingCode(CustomTrainingData data) {
//
//	std::string start = "";
//	std::string header = data.code + "-"+ std::to_string(data.numShots);
//	std::vector<int>deltaChangeInBoost;
//	std::vector<int>deltaChangeMinVelocity;
//	std::vector<int>deltaChangeMaxVelocity;
//	int pastMinVelocity = 0;
//	int pastMaxVelocity = 0;
//	int pastBoost = 0;
//	for (int boostAmount : data.boostAmounts) {
//		deltaChangeInBoost.push_back(boostAmount - pastBoost);
//	}
//	for (int minVelocity : data.startingVelocityMin) {
//		deltaChangeMinVelocity.push_back(minVelocity - pastMinVelocity);
//	}
//	for (int maxVelocity : data.startingVelocityMax) {
//		deltaChangeMaxVelocity.push_back(maxVelocity - pastMaxVelocity);
//	}
//
//
//	return "";
//}

void VersatileTraining::Render(CanvasWrapper canvas) {
	if (!editingVariances) return;

	LinearColor colors;
	colors.R = 255;
	colors.G = 255;
	colors.B = 0;
	colors.A = 255;
	canvas.SetColor(colors);

	canvas.SetPosition(Vector2F{ 0.0, 0.0 });
	canvas.DrawString("Boost Amount: " + std::to_string(tempBoostAmount), 2.0, 2.0,false);
	//canvas.SetPosition(20, 20);
	//canvas.SetColor(255, 255, 255, 255);
	//canvas.DrawString("Hello world!", 1, 1);
}