#include "pch.h"
#include "src/core/versatileTraining.h"


void VersatileTraining::registerNotifiers() {

	cvarManager->registerNotifier(
		"open_gallery",
		[this](std::vector<std::string> args) {
			cvarManager->executeCommand("togglemenu " + GetMenuName());
		}, "Displays the window for bakkes garage.", PERMISSION_ALL
	);
	//cvarManager->setBind("C", "open_gallery");

	cvarManager->registerNotifier(
		"lockScene",
		[this](std::vector<std::string> args) {
			lockScene = !lockScene;
		}, "lock training sequence to current values", PERMISSION_ALL
	);
	cvarManager->setBind("V", "lockScene");

	cvarManager->registerNotifier(
		"findController",
		[this](std::vector<std::string> args) {
			if (!isInTrainingEditor())return;

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
	cvarManager->setBind("O", "findController");
	cvarManager->registerNotifier("unlockCar", [this](std::vector<std::string> args) {
		if (!isInTrainingEditor())return;
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
		if (!isInTrainingEditor())return;
		if (currentShotState.freezeCar) {
			LOG("car unfrozen");
		}
		else {
			LOG("car frozen");
		}

		currentShotState.freezeCar = !currentShotState.freezeCar;


		}, "freeze car", PERMISSION_ALL);

	cvarManager->setBind("F", "freezeCar");

	cvarManager->registerNotifier("removeJump", [this](std::vector<std::string> args) {
		if (!isInTrainingEditor())return;
		if (currentShotState.hasJump) {
			LOG("car's jump removed");
		}
		else {
			LOG("car's jump returned");
		}
		LOG("flipped jump state");
		currentShotState.hasJump = !currentShotState.hasJump;


		}, "remove car's jump", PERMISSION_ALL);

	cvarManager->setBind("J", "removeJump");

	cvarManager->registerNotifier("lockStartingVelocity", [this](std::vector<std::string> args) {
		if (!isInTrainingEditor())return;
		
		unlockStartingVelocity = !unlockStartingVelocity;


		}, "remove car's jump", PERMISSION_ALL);

	cvarManager->setBind("B", "lockStartingVelocity");


	cvarManager->registerNotifier("printDataMap", [this](std::vector<std::string> args) {

		for (auto [key, value] : trainingData) {
			LOG("Name: {}", value.name);
			LOG("ID: {}", key);
			LOG("Code: {}", value.code);
			LOG("Num Shots: {}", value.numShots);
			for (int i = 0; i < value.numShots; i++) {


				LOG("Shot {}: Boost Amount: {}", i, value.shots[i].boostAmount);
				LOG("Shot {}: Starting Velocity: {}", i, value.shots[i].startingVelocity);
				LOG("Shot {}: Freeze Car: {}", i, static_cast<int>(value.shots[i].freezeCar));

				// Jump state - fixed to use value.shots[i] instead of currentTrainingData
				LOG("Shot {}: Has Jump: {}", i, static_cast<int>(value.shots[i].hasJump));
				LOG("Shot {} : starting velocity : {} {} {}", i, value.shots[i].extendedStartingVelocity.X, value.shots[i].extendedStartingVelocity.Y, value.shots[i].extendedStartingVelocity.Z);
				// Goal blocker positions
				LOG("Shot {}: Goal Blocker First Point: X={}, Z={}",
					i,
					static_cast<int>(value.shots[i].goalBlocker.first.X),
					static_cast<int>(value.shots[i].goalBlocker.first.Z));

				LOG("Shot {}: Goal Blocker Second Point: X={}, Z={}",
					i,
					static_cast<int>(value.shots[i].goalBlocker.second.X),
					static_cast<int>(value.shots[i].goalBlocker.second.Z));
				LOG("has recording ? {}", value.shots[i].recording.inputs.size() > 0 ? "true" : "false");
				LOG("number of inputs : {}", value.shots[i].recording.inputs.size());

				for (int j = 0; j < value.shots[i].recording.inputs.size(); j++) {
					LOG("input {}: Throttle: {:.7f}, Steer: {:.7f}, Pitch: {:.7f}, Yaw: {:.7f}, Roll: {:.7f}, DodgeForward: {:.7f}, DodgeStrafe: {:.7f}, Handbrake: {}, Jump: {}, ActivateBoost: {}, HoldingBoost: {}, Jumped: {}",
												j,
												value.shots[i].recording.inputs[j].Throttle,
												value.shots[i].recording.inputs[j].Steer,
												value.shots[i].recording.inputs[j].Pitch,
												value.shots[i].recording.inputs[j].Yaw,
												value.shots[i].recording.inputs[j].Roll,
												value.shots[i].recording.inputs[j].DodgeForward,
												value.shots[i].recording.inputs[j].DodgeStrafe,
												value.shots[i].recording.inputs[j].Handbrake ? "true" : "false",
												value.shots[i].recording.inputs[j].Jump ? "true" : "false",
												value.shots[i].recording.inputs[j].ActivateBoost ? "true" : "false",
												value.shots[i].recording.inputs[j].HoldingBoost ? "true" : "false",
												value.shots[i].recording.inputs[j].Jumped ? "true" : "false");
				}
				// Goal anchors state
				LOG("Shot {}: Goal Anchors: First={}, Second={}",
					i,
					value.shots[i].goalAnchors.first ? "true" : "false",
					value.shots[i].goalAnchors.second ? "true" : "false");
			}//i, value.boostAmounts[i], value.startingVelocity[i], value.freezeCar[i]
			LOG("------------------------------");
		}


		}, "print local data map", PERMISSION_ALL);

	cvarManager->setBind("L", "printDataMap");

	cvarManager->registerNotifier("printCurrentPack", [this](std::vector<std::string> args) {
		//if (!isInTrainingEditor())return;

		//currentTrainingData
		LOG("Name: {}", currentTrainingData.name);
		LOG("Code: {}", currentTrainingData.code);
		LOG("Num Shots: {}", currentTrainingData.numShots);
		for (int i = 0; i < currentTrainingData.numShots; i++) {
			LOG("Shot {}: Boost Amount: {}, Starting Velocity: {}, Freeze Car: {}, has jump: {}", i, currentTrainingData.shots[i].boostAmount, currentTrainingData.shots[i].startingVelocity, static_cast<int>(currentTrainingData.shots[i].freezeCar), static_cast<int>(currentTrainingData.shots[i].hasJump));
			LOG("has recording? {}", currentTrainingData.shots[i].recording.inputs.size() > 0 ? "true" : "false");
		}//i, value.boostAmounts[i], value.startingVelocity[i], value.freezeCar[i]

		}, "print local data map", PERMISSION_ALL);

	cvarManager->setBind("P", "printCurrentPack");



	cvarManager->registerNotifier("editGoalBlocker", [this](std::vector<std::string> args) {
		if (!isInTrainingEditor())return;
		if (!editingGoalBlocker) {
			LOG("Editing goal blocker");
		}
		else {
			LOG("Not editing goal blocker");
		}
		if (currentTrainingData.customPack && goalBlockerEligbleToBeEdited) {//&& isInTrainingEditor()
			LOG("flipping editing goal blocker to {}", editingGoalBlocker ? "true" : "false");
			editingGoalBlocker = !editingGoalBlocker;
		}
		else {
			LOG("not in a custom pack and picked shot, cant edit goal blocker");
		}
		}, "toggling editting goal blocker", PERMISSION_ALL);
	cvarManager->setBind("G", "editGoalBlocker");


	cvarManager->registerNotifier("spawnBot", [this](std::vector<std::string> args) {
		if (!(isInTrainingEditor() || isInTrainingPack())) return;
		shotReplicationManager.currentShotRecording = currentShotState.recording;
		shotReplicationManager.spawnBot(gameWrapper.get());

		}, "spawn bot in custom training", PERMISSION_ALL);
	cvarManager->setBind("M", "spawnBot");

	cvarManager->registerNotifier("startRecording", [this](std::vector<std::string> args) {
		if (!(isInTrainingEditor() || isInTrainingPack()) && shotReplicationManager.canSpawnBot) return;
		shotReplicationManager.startRecordingShot(gameWrapper.get());
		}, "start recording", PERMISSION_ALL);
	cvarManager->setBind("N", "startRecording");

	cvarManager->registerNotifier("currentShotState", [this](std::vector<std::string> args) {
		
		LOG("current shot state" );
		LOG("current editted shot {}",currentTrainingData.currentEditedShot);
		LOG("boost amount : {} ", currentShotState.boostAmount);
		LOG("starting velocity : {} ", currentShotState.startingVelocity);
		LOG("freeze car : {} ", currentShotState.freezeCar);
		LOG("goal blocker x1 : {}, z1 : {} x2 : {}, z2 : {} ", currentShotState.goalBlocker.first.X, currentShotState.goalBlocker.first.Z, currentShotState.goalBlocker.second.X, currentShotState.goalBlocker.second.Z);
		LOG("goal anchors first : {}, second : {} ", currentShotState.goalAnchors.first ? "true" : "false", currentShotState.goalAnchors.second ? "true" : "false");
		LOG("has jump : {} ", currentShotState.hasJump ? "true" : "false");
		LOG("has recording ? {}", currentShotState.recording.inputs.size() > 0 ? "true" : "false");
		LOG("number of inputs : {}", currentShotState.recording.inputs.size());

		}, "dump recorded inputs", PERMISSION_ALL);
	cvarManager->setBind("currentShotState", "dumpInputs");


	cvarManager->registerNotifier("printCurrentState", [this](std::vector<std::string> args) {
		if (!isInTrainingEditor())return;
		LOG("boost : {}", currentShotState.boostAmount);
		LOG("starting velocity : {}", currentShotState.startingVelocity);
		LOG("freeze car : {}", currentShotState.freezeCar);
		LOG("goal blocker x1 : {}, z1 : {} x2 : {}, z2 : {}", currentShotState.goalBlocker.first.X, currentShotState.goalBlocker.first.Z, currentShotState.goalBlocker.second.X, currentShotState.goalBlocker.second.Z);
		LOG("goal anchors first : {}, second : {}", currentShotState.goalAnchors.first ? "true" : "false", currentShotState.goalAnchors.second ? "true" : "false");
		LOG("has jump : {}", currentShotState.hasJump ? "true" : "false");


		}, "printing current state", PERMISSION_ALL);
	cvarManager->setBind("Y", "printCurrentState");

	cvarManager->registerNotifier("saveReplaySnapshot", [this](std::vector<std::string> args) {
		snapshotManager.takeSnapShot(gameWrapper.get(), focusCarID);

		}, "printing current state", PERMISSION_ALL);
	cvarManager->setBind("K", "saveReplaySnapshot");

}
