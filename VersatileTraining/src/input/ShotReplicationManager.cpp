#include "pch.h"
#include "ShotReplicationManager.h"

void ShotReplicationManager::startRecordingShot(GameWrapper* gw) {
	if (botSpawnedTest) return;

	CVarWrapper recordingEnabledCvar = _globalCvarManager->getCvar("versatile_recording_enabled");
	bool recordingFeaturesEnabled = recordingEnabledCvar ? recordingEnabledCvar.getBoolValue() : false;


	if (!recordingFeaturesEnabled) return;
	if (!startRecording) {
		if (botSpawnedTest) {
			botSpawnedTest = false;
		}
		auto car = gw->GetLocalCar();
		if (!car) return;

		
		currentShotRecording = ShotRecording();
		currentShotRecording.carBody = car.GetLoadoutBody();
		 
		currentShotRecording.settings = gw->GetSettings().GetGamepadSettings();
		
		

		currentShotRecording.startState = car.GetRBState();
		primedToStartRecording = true;
	}

}


void ShotReplicationManager::spawnBot(GameWrapper* gameWrapper) {
	CVarWrapper recordingEnabledCvar = _globalCvarManager->getCvar("versatile_recording_enabled");
	bool recordingFeaturesEnabled = recordingEnabledCvar ? recordingEnabledCvar.getBoolValue() : false;


	if (!recordingFeaturesEnabled) return;
	if (botSpawnedTest) {
		 
		return;
	}
	 
	if (currentShotRecording.inputs.empty()) {
		 
		return;
	}
	if (currentShotRecording.inputs.size() == 0) {
		 
		return;
	}

	POV pov = gameWrapper->GetCamera().GetPOV();
	auto server = gameWrapper->GetCurrentGameState();
	if (!server) return;
	frame = 0;
	ProfileCameraSettings settings = gameWrapper->GetSettings().GetCameraSettings();
	
	auto car = server.GetGameCar();
	if (!car) return;
	Vector carLoc = car.GetLocation();
	Rotator carRot = car.GetRotation();
	 
	 
	server.DestroyCars();
	gameWrapper->SetTimeout([this, settings](GameWrapper* gw) {
		auto server = gw->GetCurrentGameState();

		server.SpawnBot(currentShotRecording.carBody, "testplayers");

		auto car = server.GetGameCar();

		PlayerControllerWrapper player_controller = server.GetLocalPrimaryPlayer();
		car.SetPlayerController(player_controller);
		player_controller.SetCar(car);
		car.GetAIController().DoNothing();

		ViewTarget new_view_target;
		new_view_target.Controller = (void*)player_controller.memory_address;
		new_view_target.Target = (void*)car.memory_address;
		new_view_target.PRI = (void*)player_controller.GetPRI().memory_address;

		CameraWrapper cam = gw->GetCamera();
		cam.SetViewTarget(new_view_target);
		cam.SetFlyCamBallTargetMode();
		cam.SetFocusActor("ball");



		}, 0.05f);
	gameWrapper->SetTimeout([this, settings](GameWrapper* gw) { //needs a timeout before setting boost for some reason, if set immediately, crashes game
		auto server = gw->GetCurrentGameState();
		if (!server) return;

		auto car = server.GetGameCar();
		BoostWrapper boost = car.GetBoostComponent();
		//car.SetbDriving(false);
		boost.SetUnlimitedBoost2(true);
		car.GetPRI().SetUserCarPreferences(currentShotRecording.settings.DodgeInputThreshold, currentShotRecording.settings.SteeringSensitivity, currentShotRecording.settings.AirControlSensitivity);
		botSpawnedTest = true;
		/*if (freezeForShot) {
			car.SetLocation(currentShotRecording->initialState->location);
		}*/
		CameraWrapper cameraWrapper = gw->GetCamera();
		cameraWrapper.SetCameraSettings(settings);

		/*car.SetLocation(currentShotRecording->initialState->location);
		car.SetRotation(currentShotRecording->initialState->rotation);*/

		}, 0.5f);

	gameWrapper->SetTimeout([this, settings, pov](GameWrapper* gw) {
		canStartPlayback = true;
		auto server = gw->GetCurrentGameState();
		if (!server) return;

		auto car = server.GetGameCar();
		RBState state = car.GetRBState();

		 
		car.SetbRigidBodyWasAwake(true);
		/*car.SetbMovable(1);
		car.SetbLockLocation(0);
		car.SetTickIsDisabled(0);*/

		 
		 //currentShotRecording.startState

		/*car.SetbDriving(false);*/

		}, 0.5f);
	gameWrapper->SetTimeout([this, settings](GameWrapper* gw) {

		auto server = gw->GetCurrentGameState();
		if (!server) return;

		auto car = server.GetGameCar();

		//car.SetbDriving(true);
		INPUT inputsToSend[2] = {};
		inputsToSend[0].type = INPUT_KEYBOARD;
		inputsToSend[0].ki.wVk = 'W';
		inputsToSend[0].ki.dwFlags = 0; // Key DOWN
		inputsToSend[1].type = INPUT_KEYBOARD;
		inputsToSend[1].ki.wVk = 'W';
		inputsToSend[1].ki.dwFlags = KEYEVENTF_KEYUP; // Key UP
		SendInput(2, inputsToSend, sizeof(INPUT));
		//if (!startPlayback && canStartPlayback) {
		//	frame = 0;

		//	//roundStarted = true;
		//	startPlayback = true;
		//	canStartPlayback = false;
		//	
		//	
		//}
		}, 1.5f);//0.5 1 
}


void ShotReplicationManager::stopRecordingShot() {
	testCalledInStartRound = false;
	startRecording = false;
	roundStarted = false;
	canSpawnBot = false;
	startPlayback = false;
	botSpawnedTest = false;
}

