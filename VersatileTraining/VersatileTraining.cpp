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


	initializeCallBacks();
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
		isCarRotatable = true;
		
		});
	gameWrapper->HookEventWithCallerPost<TrainingEditorWrapper>("Function GameEvent_TrainingEditor_TA.EditorMode.StopEditing", [this](TrainingEditorWrapper cw, void* params, std::string eventName) {
		LOG("stopped editing");
		isCarRotatable = false;
		});

	gameWrapper->HookEventWithCallerPost<TrainingEditorWrapper>("Function TAGame.Ball_GameEditor_TA.EditingEnd", [this](TrainingEditorWrapper cw, void* params, std::string eventName) {
		LOG("car is being edited");
		editingVariances = true;
		});
	gameWrapper->HookEventWithCallerPost<TrainingEditorWrapper>("Function TAGame.GameEditor_Actor_TA.EditingEnd", [this](TrainingEditorWrapper cw, void* params, std::string eventName) {
		LOG("ball is being edited");
		editingVariances = false;
		});
	//TAGame.GameEvent_TrainingEditor_TA.EventPlaytestStarted
	/*gameWrapper->HookEvent("Function TAGame.GameEvent_TrainingEditor_TA.EventPlaytestStarted", [this](std::string eventName) {
		isCarRotatable = false;

		});*/
	gameWrapper->HookEvent("Function TAGame.GameEvent_TrainingEditor_TA.EndPlayTest", [this](std::string eventName) {
		isCarRotatable = true;

		});
	gameWrapper->HookEventWithCallerPost<CarWrapper>("Function TAGame.CameraState_CarRef_TA.GetCarRotation", [this](CarWrapper cw, void* params, std::string eventName) {
		//LOG("playtest started");
		if (!cw || cw.IsNull()|| !isCarRotatable) return;
		if (changeCarSpawnRotation()) {
			isCarRotatable = false;
		}
		});
	//TAGame.PlayerController_TA.EventTrainingEditorActorModified
	gameWrapper->HookEvent("Function TAGame.PlayerController_TA.EventTrainingEditorActorModified",
		[this](std::string eventName) {
			if (editingVariances) {
				//LOG("Training editor actor modified");
				//checkForR1Press();
				checkForButtonPress(5);
				checkForButtonPress(6);
			}
		}
	);

	

		
}

bool VersatileTraining::changeCarSpawnRotation() {
	auto serv = gameWrapper->GetGameEventAsServer();
	if (!serv) return 0;
	auto car = serv.GetGameCar();
	Rotator rot = car.GetRotation();
	LOG("Car rotation: {}", rot.Pitch);
	LOG("Car rotation: {}", rot.Yaw);
	LOG("Car rotation: {}", rot.Roll);
	rot.Yaw += 23036;
	car.SetCarRotation(rot);

	Rotator newRot = car.GetRotation();
	LOG("Car rotation: {}", newRot.Pitch);
	LOG("Car rotation: {}", newRot.Yaw);
	LOG("Car rotation: {}", newRot.Roll);
	LOG("Car rotation changed");
	return 1;
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