#include "pch.h"
#include "src/core/VersatileTraining.h"


BAKKESMOD_PLUGIN(VersatileTraining, "write a plugin description here", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void VersatileTraining::onLoad()
{
	_globalCvarManager = cvarManager;
	 
	LOG("Plugin loaded!!");
	
	this->loadHooks();
	registerNotifiers();
	gameWrapper->RegisterDrawable([this](CanvasWrapper canvas) {
		Render(canvas);
	});
	//
	middleMouseIndex = gameWrapper->GetFNameIndexByString("MiddleMouseButton");
	gameWrapper->HookEvent("Function Engine.GameViewportClient.Tick",
		std::bind(&VersatileTraining::onTick, this, std::placeholders::_1));


	controllerManager.initializeCallBacks();

	std::filesystem::path myDataFolder = gameWrapper->GetDataFolder() / "VersatileTraining";
	saveFilePath = myDataFolder / "packs.txt";

	trainingData = LoadCompressedTrainingData(saveFilePath);
	for (auto& [key, value] : trainingData) {
		shiftToNegative(value);

		
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

void VersatileTraining::loadHooks() {
	setupTrainingEditorHooks();
	setupTrainingShotHooks();
	setupEditorMovementHooks();
	setupGoalBlockerHooks();
	setupInputHandlingHooks();
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

			controllerManager.enumerateControllers();
		}, "check to see if there's a connected controller", PERMISSION_ALL
	);
	cvarManager->setBind("O", "unlockCar");
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

		if (currentShotState.freezeCar) {
			LOG("car unfrozen");
		}
		else {
			LOG("car frozen");
		}
	
		currentShotState.freezeCar = !currentShotState.freezeCar;


		}, "freeze car", PERMISSION_ALL);

	cvarManager->setBind("F", "freezeCar");

	cvarManager->registerNotifier("printDataMap", [this](std::vector<std::string> args) {

		for (auto [key, value] : trainingData) {
			LOG("Name: {}", value.name);
			LOG("Code: {}", key);
			LOG("Num Shots: {}", value.numShots);
			for (int i = 0; i < value.numShots; i++) {
				LOG("Shot {}: Boost Amount: {}, Starting Velocity: {}, Freeze Car: {}. goal blocker x1 {}, z1 {} x2 {}, z2 {}", i, value.shots[i].boostAmount, value.shots[i].startingVelocity,
					static_cast<int>(value.shots[i].freezeCar),static_cast<int>(value.shots[i].goalBlocker.first.X), static_cast<int>(value.shots[i].goalBlocker.first.Z),
					static_cast<int>(value.shots[i].goalBlocker.second.X), static_cast<int>(value.shots[i].goalBlocker.second.Z));
				LOG("goal anchors first: {}, second: {}", value.shots[i].goalAnchors.first ? "true" : "false", value.shots[i].goalAnchors.second ? "true" : "false");
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
			LOG("Shot {}: Boost Amount: {}, Starting Velocity: {}, Freeze Car: {}", i, currentTrainingData.shots[i].boostAmount, currentTrainingData.shots[i].startingVelocity, static_cast<int>(currentTrainingData.shots[i].freezeCar));
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
		if (currentTrainingData.customPack && goalBlockerEligbleToBeEdited ) {//&& gameWrapper->IsInCustomTraining()
			LOG("flipping editing goal blocker to {}", editingGoalBlocker ? "true" : "false");
			editingGoalBlocker = !editingGoalBlocker;
		}
		else {
			LOG("not in a custom pack and picked shot, cant edit goal blocker");
		}
		}, "toggling editting goal blocker", PERMISSION_ALL);
	cvarManager->setBind("G", "editGoalBlocker");


	cvarManager->registerNotifier("spawnBot", [this](std::vector<std::string> args) {
		shotReplicationManager.spawnBot(gameWrapper.get());

		}, "spawn bot in custom training", PERMISSION_ALL);
	cvarManager->setBind("M", "spawnBot");

	cvarManager->registerNotifier("startRecording", [this](std::vector<std::string> args) {
		shotReplicationManager.startRecordingShot(gameWrapper.get());
		}, "start recording", PERMISSION_ALL);
	cvarManager->setBind("N", "startRecording");

	cvarManager->registerNotifier("dumpInputs", [this](std::vector<std::string> args) {

		if (shotReplicationManager.currentShotRecording == nullptr) {
			LOG("no inputs to dump");
			return;
		}
		struct ControllerInput {
			float Throttle = .0f;
			float Steer = .0f;
			float Pitch = .0f;
			float Yaw = .0f;
			float Roll = .0f;
			float DodgeForward = .0f;
			float DodgeStrafe = .0f;
			unsigned long Handbrake : 1;
			unsigned long Jump : 1;
			unsigned long ActivateBoost : 1;
			unsigned long HoldingBoost : 1;
			unsigned long Jumped : 1;
		};
		for (const auto &input : shotReplicationManager.currentShotRecording->inputs) {
			LOG("Throttle: {:.7f}, Steer: {:.7f}, Pitch: {:.7f}, Yaw: {:.7f}, Roll: {:.7f}, DodgeForward: {:.7f}, DodgeStrafe: {:.7f}, Handbrake: {}, Jump: {}, ActivateBoost: {}, HoldingBoost: {}, Jumped: {}",
								input.Throttle, input.Steer, input.Pitch, input.Yaw, input.Roll, input.DodgeForward, input.DodgeStrafe,
								input.Handbrake? "true": "false", input.Jump ? "true" : "false", input.ActivateBoost ? "true" : "false", input.HoldingBoost ? "true" : "false", input.Jumped ? "true" : "false");
		}
		}, "dump recorded inputs", PERMISSION_ALL);
	cvarManager->setBind("I", "dumpInputs");
}






void VersatileTraining::onUnload() {
	LOG("Unloading Versatile Training");
	for (auto& [key, value] : trainingData) {
		shiftToPositive(value);
	}
	SaveCompressedTrainingData(trainingData, saveFilePath);
	CleanUp();
}

void VersatileTraining::CleanUp() {
	for (auto& controller : controllerManager.controllers) {
		if (controller) {
			controller->Unacquire(); // Unacquire before release
			controller->Release();   // Release the DirectInput device
			controller = nullptr;
		}
	}
	controllerManager.controllers.clear(); // Clear the vector
	LOG("All controllers cleaned up.");

	if (controllerManager.dinput) {
		controllerManager.dinput->Release();
		controllerManager.dinput = nullptr;
		LOG("DirectInput object released.");
	}
}



void VersatileTraining::onTick(std::string eventName) {
	
	if (editingGoalBlocker) {
		if (gameWrapper->IsKeyPressed(middleMouseIndex) && middleMouseReleased) {
			saveCursorPos = true;
			middleMouseReleased = false;
			LOG("middle mouse button pressed");
		}
		else if (!gameWrapper->IsKeyPressed(middleMouseIndex)) {
			middleMouseReleased = true;
		}
	}

	if (!gameWrapper->IsInCustomTraining())return;


	if(!currentTrainingData.customPack) return;
	if (!currentShotState.goalAnchors.first  || !currentShotState.goalAnchors.second) return;

	ServerWrapper gameState = gameWrapper->GetCurrentGameState();

	BallWrapper ball = gameState.GetBall();

	if (!ball) {
		return;
	}

	ArrayWrapper<GoalWrapper> allGoals = gameState.GetGoals();

	if (allGoals.Count() < 2) {
		return;
	}

	Vector ballPosition = ball.GetLocation();
	Vector ballVelocity = ball.GetVelocity();

	if (ballPosition.Y > backWall && ballVelocity.Y > 0) {

		if (inRectangle(currentShotState.goalBlocker, ballPosition)) {
			GoalWrapper enemyGoal = allGoals.Get(1);

			if (!enemyGoal) {
				return;
			}

			Vector explosionPosition = ballPosition;

			ball.eventOnHitGoal(enemyGoal, explosionPosition);
			return;
		}

		ballVelocity.Y = -ballVelocity.Y;
		ball.SetVelocity(ballVelocity);
	}
}



