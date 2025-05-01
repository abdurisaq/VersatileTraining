#include "pch.h"
#include "ShotReplicationManager.h"

void ShotReplicationManager::startRecordingShot(GameWrapper* gw) {
	if (botSpawnedTest) return;
	if (!startRecording) {
		if (botSpawnedTest) {
			botSpawnedTest = false;
		}
		currentShotRecording = std::make_shared<ShotRecording>();
		currentShotRecording->carBody = gw->GetLocalCar().GetLoadoutBody();
		currentShotRecording->settings = gw->GetSettings().GetGamepadSettings();


		LOG("settings controllerDeadzone {}, DodgeInputThreshold {}, SteeringSensitivity {} , AirControlSensitivity {}", currentShotRecording->settings.ControllerDeadzone, currentShotRecording->settings.DodgeInputThreshold, currentShotRecording->settings.SteeringSensitivity, currentShotRecording->settings.AirControlSensitivity);
		// Record initial state
		auto car = gw->GetLocalCar();
		currentShotRecording->initialState = std::make_shared<ShotRecording::InitialState>();
		currentShotRecording->initialState->velocity = car.GetVelocity();
		currentShotRecording->initialState->location = car.GetLocation();
		currentShotRecording->initialState->rotation = car.GetRotation();

		currentShotRecording->startState = car.GetRBState();
		primedToStartRecording = true;
	}

}


void ShotReplicationManager::spawnBot(GameWrapper* gameWrapper) {
	if (botSpawnedTest) {
		LOG("bot already spawned");
		return;
	}
	if (currentShotRecording == nullptr) return;
	if (currentShotRecording->inputs.size() == 0) {
		LOG("no inputs to play");
		return;
	}

	POV pov = gameWrapper->GetCamera().GetPOV();
	auto server = gameWrapper->GetCurrentGameState();
	if (!server) return;
	frame = 0;
	ProfileCameraSettings settings = gameWrapper->GetSettings().GetCameraSettings();
	std::vector<std::pair<std::string, std::string>> bindings = gameWrapper->GetSettings().GetAllPCBindings();
	for (auto& binding : bindings) {
		LOG("binding: {} : {}", binding.first, binding.second);
	}
	auto car = server.GetGameCar();
	Vector carLoc = car.GetLocation();
	Rotator carRot = car.GetRotation();
	LOG(" car location: {:.7f}, {:.7f}, {:.7f}", car.GetLocation().X, car.GetLocation().Y, car.GetLocation().Z);
	LOG(" rotation : {}, {}, {}", car.GetRotation().Pitch, car.GetRotation().Yaw, car.GetRotation().Roll);
	server.DestroyCars();
	gameWrapper->SetTimeout([this, settings](GameWrapper* gw) {
		auto server = gw->GetCurrentGameState();

		server.SpawnBot(this->currentShotRecording->carBody, "testplayers");

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
		car.GetPRI().SetUserCarPreferences(currentShotRecording->settings.DodgeInputThreshold, currentShotRecording->settings.SteeringSensitivity, currentShotRecording->settings.AirControlSensitivity);
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

		/*CameraWrapper cameraWrapper = gw->GetCamera();
		cameraWrapper.SetCameraState("CameraState_BallCam_TA");  doesnt work
		cameraWrapper.SetPOV(pov);*/

		ServerWrapper sw = gw->GetGameEventAsServer();
		if (!sw) return;
		GameEditorWrapper training = GameEditorWrapper(sw.memory_address);
		LOG("awake: {} ", car.GetbRigidBodyWasAwake());
		RBState playerState;
		playerState.Location = currentShotRecording->initialState->location;
		playerState.Quaternion = RotatorToQuat(currentShotRecording->initialState->rotation);
		car.SetPhysicsState(currentShotRecording->startState);



		}, 0.5f);
	gameWrapper->SetTimeout([this, settings](GameWrapper* gw) {
		if (!startPlayback && canStartPlayback) {
			frame = 0;

			//roundStarted = true;
			startPlayback = true;
			canStartPlayback = false;
			
			
		}
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

