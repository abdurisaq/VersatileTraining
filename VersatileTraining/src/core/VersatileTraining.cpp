#include "pch.h"
#include "src/core/VersatileTraining.h"

BAKKESMOD_PLUGIN(VersatileTraining, "Train Like Never Before", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void VersatileTraining::onLoad()
{
	_globalCvarManager = cvarManager;
	 

	LOG("Plugin loaded!!");
	trainingData = std::make_shared<std::unordered_map<std::string, CustomTrainingData>>();
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

	 myDataFolder = gameWrapper->GetDataFolder() / "VersatileTraining";

	 if (!std::filesystem::exists(myDataFolder)) {
		 try {
			 if (std::filesystem::create_directories(myDataFolder)) {
				 LOG("Plugin data folder created: {}", myDataFolder.string());
				 firstTime = true;
				 
				 
				
				 
			 }
			 else {
				 
				 LOG("Plugin data folder may have been created by another process or already existed: {}", myDataFolder.string());
			 }
		 }
		 catch (const std::filesystem::filesystem_error& e) {
			 LOG("FATAL: Error creating plugin data folder {}: {}. Plugin functionality will be severely limited.", myDataFolder.string(), e.what());
			 return; 
		 }
	 }
	 else {
		 firstTime = false;
	 }
	storageManager.saveTrainingFilePath = myDataFolder / "packs.txt";
	storageManager.saveReplayStateFilePath = myDataFolder / "replayStates.txt";

	cvarManager->registerCvar("versatile_recording_enabled", "0",
		"Enable experimental recording & playback features", true, true, 0, true, 1)
		.addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
		recordingEnabled = cvar.getBoolValue();
		});

	snapshotManager.replayStates = storageManager.loadReplayStates(storageManager.saveReplayStateFilePath);
	
	storageManager.packOverrideSettings = storageManager.loadPackOverrideSettings(myDataFolder);

	readCurrentBindings();

	specialKeybinds = storageManager.loadSpecialKeybinds(myDataFolder);
	bool hasBinds = false;
	for (const auto& command : { "unlockCar", "freezeCar", "removeJump" }) {
		if (currentBindings.find(command) != currentBindings.end()) {
			LOG("Found existing binding for command: {}", command);
			hasBinds = true;
			break;
		}
	}

	if (!hasBinds) {
		LOG("No existing bindings found, setting defaults");
		SetDefaultKeybinds();
	}
	currentBindings["Roll Left"] = getKeyName(specialKeybinds.rollLeft);
	currentBindings["Roll Right"] = getKeyName(specialKeybinds.rollRight);
	currentBindings["Decrease Boost"] = getKeyName(specialKeybinds.decreaseBoost);
	currentBindings["Increase Boost"] = getKeyName(specialKeybinds.increaseBoost);
	currentBindings["Decrease Velocity"] = getKeyName(specialKeybinds.decreaseVelocity);
	currentBindings["Increase Velocity"] = getKeyName(specialKeybinds.increaseVelocity);

	

	*trainingData = storageManager.loadCompressedTrainingDataWithRecordings(myDataFolder);
	for (auto& [key, value] : *trainingData) {
		shiftToNegative(value);
	}

	serverRunning = true;
	

	allSnapshotsSearchBuffer[0] = '\0';
    allSnapshotsFilterType = 0;
    allSnapshotsSourceFilter = 0;
    allSnapshotsSortAscending = true;
    LoadSnapshotGroups();
	// Start the server thread with the shared data

	serverThread = std::thread(&VersatileTraining::runServer, this, &serverRunning, gameWrapper->GetUniqueID().str(), trainingData, myDataFolder,std::ref(hasAction), std::ref(pendingAction), std::ref(pendingActionMutex));

	
}

void VersatileTraining::loadHooks() {
	setupTrainingEditorHooks();
	setupTrainingShotHooks();
	setupEditorMovementHooks();
	setupGoalBlockerHooks();
	setupInputHandlingHooks();
	replayHooks();
}








void VersatileTraining::onUnload() {
	LOG("Unloading Versatile Training");

	storageManager.saveReplayStates( snapshotManager.replayStates, storageManager.saveReplayStateFilePath);

	
	for (auto& [key, value] : *trainingData) {
		shiftToPositive(value);
	}
	storageManager.saveCompressedTrainingDataWithRecordings(*trainingData, myDataFolder);
	
	storageManager.saveSpecialKeybinds(specialKeybinds, myDataFolder);


	storageManager.savePackOverrideSettings(storageManager.packOverrideSettings, myDataFolder);
	CleanUp();

	serverRunning = false;
	if (serverThread.joinable()) {
		serverThread.join();
		LOG("Server thread joined.");
	}
	else {
		LOG("Server thread not joinable.");
	}
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
	checkPendingActions();
	if (!(isInTrainingEditor() || isInTrainingPack())) return;

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



void VersatileTraining::checkPendingActions() {
	if (hasAction.load(std::memory_order_acquire)) {
		std::string action;
		{
			std::lock_guard<std::mutex> lock(pendingActionMutex);
			action = pendingAction;
			pendingAction.clear();
			hasAction.store(false, std::memory_order_release);
		}

		if (action == "RELOAD") {
			// Reload all training packs
			LOG("Reloading all training packs from disk");

			*trainingData = storageManager.loadCompressedTrainingDataWithRecordings(myDataFolder);
			for (auto& [key, value] : *trainingData) {
				shiftToNegative(value);
			}
			
		}
		else if (!action.empty()) {
			LOG("Reloading all training packs from disk");
			HWND rocketLeagueWindow = FindWindowA("LaunchUnrealUWindowsClient", "Rocket League (64-bit, DX11, Cooked)");
			if (rocketLeagueWindow != NULL) {
					
					if (IsIconic(rocketLeagueWindow)) {
							ShowWindow(rocketLeagueWindow, SW_RESTORE);
					}
					
					SetForegroundWindow(rocketLeagueWindow);
					LOG("Focused Rocket League window");
			} else {
					LOG("Could not find Rocket League window");
			}

			*trainingData = storageManager.loadCompressedTrainingDataWithRecordings(myDataFolder);
			for (auto& [key, value] : *trainingData) {
				shiftToNegative(value);
			}
			
			gameWrapper->SetTimeout([this, action](GameWrapper* gw) {
				cvarManager->executeCommand("load_training " + action);
				}, 1.5f);
			//probably the reason, needs a sec after full screening
			
		}
	}

}