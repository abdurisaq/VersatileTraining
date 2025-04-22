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
	registerNotifiers();
	gameWrapper->RegisterDrawable([this](CanvasWrapper canvas) {
		Render(canvas);
	});

	middleMouseIndex = gameWrapper->GetFNameIndexByString("MiddleMouseButton");
	gameWrapper->HookEvent("Function Engine.GameViewportClient.Tick",
		std::bind(&VersatileTraining::onTick, this, std::placeholders::_1));


	initializeCallBacks();

	std::filesystem::path myDataFolder = gameWrapper->GetDataFolder() / "VersatileTraining";
	saveFilePath = myDataFolder / "packs.txt";

	trainingData = LoadCompressedTrainingData(saveFilePath);
	for (auto& [key, value] : trainingData) {
		shiftVelocitiesToNegative(value.startingVelocity);
	}

	//		
	//	}, "button pressed", PERMISSION_ALL
	//	);
	//	cvarManager->setBind(input.first, input.first + "pressed");
	//}
	
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
void VersatileTraining::registerNotifiers() {

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
	cvarManager->registerNotifier("unlockCar", [this](std::vector<std::string> args) {

		if (lockRotation) {
			LOG("car unlocked");
		}
		else {
			LOG("car locked");
		}
		lockRotation = !lockRotation;


		}, "unlock car", PERMISSION_ALL);

	cvarManager->setBind("X", "unlockCar");
	cvarManager->registerNotifier("freezeCar", [this](std::vector<std::string> args) {

		if (freezeCar) {
			LOG("car unfrozen");
		}
		else {
			LOG("car frozen");
		}
		freezeCar = !freezeCar;
		if (currentTrainingDataUsed != nullptr) {
			//currentTrainingDataUsed->freezeCar[currentTrainingDataUsed->currentEditedShot] = freezeCar;
			currentTrainingData.freezeCar[currentTrainingData.currentEditedShot] = freezeCar;
		}


		}, "freeze car", PERMISSION_ALL);

	cvarManager->setBind("F", "freezeCar");

	cvarManager->registerNotifier("printDataMap", [this](std::vector<std::string> args) {

		for (auto [key, value] : trainingData) {
			LOG("Name: {}", value.name);
			LOG("Code: {}", key);
			LOG("Num Shots: {}", value.numShots);
			for (int i = 0; i < value.numShots; i++) {
				LOG("Shot {}: Boost Amount: {}, Starting Velocity: {}, Freeze Car: {}", i, value.boostAmounts[i], value.startingVelocity[i], static_cast<int>(value.freezeCar[i]));
			}//i, value.boostAmounts[i], value.startingVelocity[i], value.freezeCar[i]
			LOG("------------------------------");
		}


		}, "print local data map", PERMISSION_ALL);

	cvarManager->setBind("L", "printDataMap");

	cvarManager->registerNotifier("printCurrentPack", [this](std::vector<std::string> args) {

		//currentTrainingData
		LOG("Name: {}", currentTrainingData.name);
		LOG("Code: {}", currentPackKey);
		LOG("Num Shots: {}", currentTrainingData.numShots);
		for (int i = 0; i < currentTrainingData.numShots; i++) {
			LOG("Shot {}: Boost Amount: {}, Starting Velocity: {}, Freeze Car: {}", i, currentTrainingData.boostAmounts[i], currentTrainingData.startingVelocity[i], static_cast<int>(currentTrainingData.freezeCar[i]));
		}//i, value.boostAmounts[i], value.startingVelocity[i], value.freezeCar[i]

		}, "print local data map", PERMISSION_ALL);

	cvarManager->setBind("P", "printCurrentPack");



	cvarManager->registerNotifier("editGoalBlocker", [this](std::vector<std::string> args) {
		
		if (!editingGoalBlocker) {
			LOG("Editing goal blocker");
		}
		else{
			LOG("Not editing goal blocker");
		}


		if (currentTrainingData.customPack && goalBlockerEligbleToBeEdited) {
			LOG("flipping editing goal blocker to {}", editingGoalBlocker ? "true" : "false");
			editingGoalBlocker = !editingGoalBlocker;
		}
		else {
			LOG("not in a custom pack and picked shot, cant edit goal blocker");
		}

		}, "toggling editting goal blocker", PERMISSION_ALL);
	cvarManager->setBind("G", "editGoalBlocker");

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


void VersatileTraining::getTrainingData(ActorWrapper cw, void* params, std::string eventName) {



	auto tw = ((TrainingEditorWrapper)cw.memory_address);
	GameEditorSaveDataWrapper data = tw.GetTrainingData();
	TrainingEditorSaveDataWrapper td = data.GetTrainingData();
	LOG("Training pack code: {}", td.GetCode().ToString());
	std::string name = td.GetTM_Name().ToString();
	LOG("Training pack name", name);
	/*GetCreatorName();
	GetDescription();*/
	LOG("Training pack type: {}", td.GetType());
	LOG("Training pack difficulty: {}", td.GetDifficulty());
	LOG("Training pack creator name: {}", td.GetCreatorName().ToString());
	LOG("Training pack description: {}", td.GetDescription().ToString());
	LOG("training pack created at: {}", td.GetCreatedAt());
	int totalRounds = tw.GetTotalRounds();
	LOG("Training num rounds: {}", totalRounds);

	int currentShot = tw.GetActiveRoundNumber();
	LOG("Training current shot : {}", currentShot);

	std::string code = td.GetCode().ToString();

	bool found = false;
	for (auto& [key, value] : trainingData) {
		if (value.name == name) {
			currentTrainingData = value;
			currentTrainingData.customPack = true;
			LOG("setting active boost amount to {}", currentTrainingData.boostAmounts[currentShot]);
			tempBoostAmount = currentTrainingData.boostAmounts[currentShot];
			tempStartingVelocity = currentTrainingData.startingVelocity[currentShot];
			LOG("setting active starting velocity to {}", tempStartingVelocity);
			currentTrainingData.currentEditedShot = currentShot;
			freezeCar = currentTrainingData.freezeCar[currentShot];
			if (freezeCar) {
				LOG("car is frozen");
			}
			else {
				LOG("car is not frozen");
			}
			if (tempBoostAmount == 101) {
				cvarManager->executeCommand("sv_training_limitboost -1");
			}
			else {
				cvarManager->executeCommand("sv_training_limitboost " + std::to_string(tempBoostAmount));
			}
			found = true;
		}
	}
	if (!found) {
		LOG("didn't find this traini pack in training data");
		currentTrainingData.initCustomTrainingData(totalRounds, name);
		currentTrainingData.customPack = false;
		cvarManager->executeCommand("sv_training_limitboost -1");
	}

	
}



void VersatileTraining::onUnload() {
	LOG("Unloading Versatile Training");
	for (auto& [key, value] : trainingData) {
		shiftVelocitiesToPositive(value.startingVelocity);
	}
	SaveCompressedTrainingData(trainingData, saveFilePath);
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



void VersatileTraining::onTick(std::string eventName) {
	
	if (editingGoalBlocker) {
		if (gameWrapper->IsKeyPressed(middleMouseIndex)) {
			saveCursorPos = true;
			LOG("middle mouse button pressed");
		}
	}
}


void VersatileTraining::Render(CanvasWrapper canvas) {
	if (editingVariances) {

		LinearColor colors;
		colors.R = 255;
		colors.G = 255;
		colors.B = 0;
		colors.A = 255;
		canvas.SetColor(colors);

		canvas.SetPosition(Vector2F{ 0.0, 0.0 });
		canvas.DrawString("Boost Amount: " + std::to_string(tempBoostAmount), 2.0, 2.0, false);
		canvas.SetPosition(Vector2F{ 0.0, 20.0 });
		if (lockRotation) {
			canvas.DrawString("Car rotation locked, press X to unlock", 2.0, 2.0, false);
		}
		else {
			canvas.DrawString("Car rotation unlocked, press X to lock", 2.0, 2.0, false);
		}
		canvas.SetPosition(Vector2F{ 0.0, 40.0 });
		if (freezeCar) {

			canvas.DrawString("Car frozen, press F to unfreeze", 2.0, 2.0, false);
		}
		else {

			canvas.DrawString("Car unfrozen, press F to freeze", 2.0, 2.0, false);
		}
		canvas.SetPosition(Vector2F{ 0.0, 60.0 });

		canvas.DrawString("Starting Velocity: " + std::to_string(tempStartingVelocity), 2.0, 2.0, false);
	}
	else if (editingGoalBlocker) {
		LOG("drawing cursor");
		Vector2 screenSize = canvas.GetSize();

		// Calculate center
		Vector2 center(screenSize.X / 2, screenSize.Y / 2);

		// Set color to white with full opacity
		canvas.SetColor(255, 255, 255, 255);

		// Size of the cross lines
		float length = 10.0f;

		// Draw horizontal line
		canvas.DrawLine(
			Vector2(center.X - length, center.Y),
			Vector2(center.X + length, center.Y),
			1.5f
		);

		// Draw vertical line
		canvas.DrawLine(
			Vector2(center.X, center.Y - length),
			Vector2(center.X, center.Y + length),
			1.5f
		);
	}


}

