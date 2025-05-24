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

	

	cvarManager->registerNotifier("lockStartingVelocity", [this](std::vector<std::string> args) {
		if (!isInTrainingEditor())return;
		
		if (!unlockStartingVelocity) {
			currentShotState.extendedStartingAngularVelocity = Vector(0, 0, 0);
		}
		
		unlockStartingVelocity = !unlockStartingVelocity;


		}, "unlock velocity", PERMISSION_ALL);

	







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
	


	cvarManager->registerNotifier("spawnBot", [this](std::vector<std::string> args) {
		if (!(isInTrainingEditor() || isInTrainingPack())) return;
		shotReplicationManager.currentShotRecording = currentShotState.recording;
		shotReplicationManager.spawnBot(gameWrapper.get());

		}, "spawn bot in custom training", PERMISSION_ALL);


	cvarManager->registerNotifier("startRecording", [this](std::vector<std::string> args) {
		if (!(isInTrainingEditor()) && shotReplicationManager.canSpawnBot) return;
		shotReplicationManager.startRecordingShot(gameWrapper.get());
		}, "start recording", PERMISSION_ALL);
	

	

	cvarManager->registerNotifier("saveReplaySnapshot", [this](std::vector<std::string> args) {
		snapshotManager.takeSnapShot(gameWrapper.get(), focusCarID);

		}, "saving a scenario from a replay / training pack", PERMISSION_ALL);
	

}
