#include "pch.h"
#include "src/core/versatileTraining.h"


void VersatileTraining::registerNotifiers() {

	cvarManager->registerNotifier(
		"open_gallery",
		[this](std::vector<std::string> args) {
			cvarManager->executeCommand("togglemenu " + GetMenuName());
		}, "Displays the window for bakkes garage.", PERMISSION_ALL
	);
	cvarManager->setBind("C", "open_gallery");

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

	cvarManager->registerNotifier("printDataMap", [this](std::vector<std::string> args) {

		for (auto [key, value] : trainingData) {
			LOG("Name: {}", value.name);
			LOG("Code: {}", key);
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
		if (!isInTrainingEditor())return;

		//currentTrainingData
		LOG("Name: {}", currentTrainingData.name);
		LOG("Code: {}", currentPackKey);
		LOG("Num Shots: {}", currentTrainingData.numShots);
		for (int i = 0; i < currentTrainingData.numShots; i++) {
			LOG("Shot {}: Boost Amount: {}, Starting Velocity: {}, Freeze Car: {}, has jump: {}", i, currentTrainingData.shots[i].boostAmount, currentTrainingData.shots[i].startingVelocity, static_cast<int>(currentTrainingData.shots[i].freezeCar), static_cast<int>(currentTrainingData.shots[i].hasJump));
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
		shotReplicationManager.spawnBot(gameWrapper.get());

		}, "spawn bot in custom training", PERMISSION_ALL);
	cvarManager->setBind("M", "spawnBot");

	cvarManager->registerNotifier("startRecording", [this](std::vector<std::string> args) {
		if (!(isInTrainingEditor() || isInTrainingPack()) && shotReplicationManager.canSpawnBot) return;
		shotReplicationManager.startRecordingShot(gameWrapper.get());
		}, "start recording", PERMISSION_ALL);
	cvarManager->setBind("N", "startRecording");

	cvarManager->registerNotifier("dumpInputs", [this](std::vector<std::string> args) {
		if (!isInTrainingEditor())return;
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
		for (const auto& input : shotReplicationManager.currentShotRecording->inputs) {
			LOG("Throttle: {:.7f}, Steer: {:.7f}, Pitch: {:.7f}, Yaw: {:.7f}, Roll: {:.7f}, DodgeForward: {:.7f}, DodgeStrafe: {:.7f}, Handbrake: {}, Jump: {}, ActivateBoost: {}, HoldingBoost: {}, Jumped: {}",
				input.Throttle, input.Steer, input.Pitch, input.Yaw, input.Roll, input.DodgeForward, input.DodgeStrafe,
				input.Handbrake ? "true" : "false", input.Jump ? "true" : "false", input.ActivateBoost ? "true" : "false", input.HoldingBoost ? "true" : "false", input.Jumped ? "true" : "false");
		}
		}, "dump recorded inputs", PERMISSION_ALL);
	cvarManager->setBind("I", "dumpInputs");


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
		if (isInReplay) {

			ReplayServerWrapper serverReplay = gameWrapper->GetGameEventAsReplay();
			if (serverReplay.IsNull()) {
				LOG("Server replay is null1");
				return;
			}
			ReplayWrapper replay = serverReplay.GetReplay();
			if (replay.IsNull()) {
				LOG("Replay is null2");
				return;
			}
			savedReplayState.replayName = replay.GetReplayName().ToString();

			savedReplayState.replayTime = replay.GetDate().ToString();

			ArrayWrapper<CarWrapper> cars = serverReplay.GetCars();
			bool found = false;
			for (auto car : cars) {
				if (car.GetPRI().GetUniqueIdWrapper().str() == focusCarID) {
					LOG("car location: {:.7f}, {:.7f}, {:.7f}", car.GetLocation().X, car.GetLocation().Y, car.GetLocation().Z);
					savedReplayState.carLocation = car.GetLocation();
					savedReplayState.carVelocity = car.GetVelocity();
					savedReplayState.carAngularVelocity = car.GetAngularVelocity();
					savedReplayState.carRotation = car.GetRotation();
					savedReplayState.focusPlayerName = car.GetOwnerName();

					BoostWrapper boost = car.GetBoostComponent();
					if (boost.IsNull()) {
						LOG("Boost component is null");
						return;
					}
					
					savedReplayState.boostAmount = static_cast<int>(boost.GetCurrentBoostAmount() * 100.0f);
					savedReplayState.boosting = boost.GetbActive();

					LOG("boost amount : {}", savedReplayState.boostAmount);
					LOG("boosting? : {}", boost.GetbActive()? "true" : "false");

					JumpComponentWrapper jump = car.GetJumpComponent();

					if (jump.IsNull()) {
						LOG("Jump component is null");
					}
					

					savedReplayState.jumpTimer = !car.GetbJumped() ||  car.GetJumpComponent().GetInactiveTime();
					savedReplayState.hasJump = jump.GetbActive(); // this is called the second the car jumps set to 1, and then immediately set to 0.
					//i guess in the tick function, ill need to continuously check for it getting set to 1, and then save the time in game it happened, and if the snapshot is saved before 
					//1.25 seconds after in game time, then they have a jump, and dodge timer is set to the difference
					
					LOG("jump inactive timer : {}", car.GetJumpComponent().GetInactiveTime());
					LOG("double jumped? : {}", car.GetbDoubleJumped() ? "true" : "false");
					LOG("jumped? : {}", savedReplayState.hasJump ? "true" : "false");
					found = true;
					break;
				}
			}

			BallWrapper ball = serverReplay.GetBall();
			if (ball.IsNull()) {
				LOG("Ball is null");
				return;
			}

			
			savedReplayState.ballLocation = ball.GetLocation();
			savedReplayState.setBallStartingRotationAndStrength(ball.GetVelocity());

			auto now = std::chrono::system_clock::now();
			std::time_t now_time = std::chrono::system_clock::to_time_t(now);

			std::tm local_tm;
			#ifdef _WIN32
				localtime_s(&local_tm, &now_time); // Windows-safe version
			#else
				localtime_r(&now_time, &local_tm); // POSIX-safe version
			#endif	

			std::ostringstream oss;
			oss << std::put_time(&local_tm, "%d-%m-%Y %H:%M:%S");
			savedReplayState.formattedTimeStampOfSaved = oss.str();
			std::ostringstream oss1;
			int secondsRemaining = static_cast<int>(gameWrapper->GetCurrentGameState().GetSecondsRemaining());
			int minutes = secondsRemaining / 60;
			int seconds = secondsRemaining % 60;
			oss1 << std::setw(2) << std::setfill('0') << minutes
				<< ":" << std::setw(2) << std::setfill('0') << seconds;
			savedReplayState.timeRemainingInGame = oss1.str();
			LOG("saved replay name : {}", savedReplayState.replayName);
			LOG("saved replay player name : {}", savedReplayState.focusPlayerName);
			LOG("saved replay player boost amount : {}", savedReplayState.boostAmount);
			LOG("saved replay date : {}", savedReplayState.replayTime);
			LOG("saved replay time stamp : {}", savedReplayState.formattedTimeStampOfSaved);
			LOG("saved replay time remaining : {}", savedReplayState.timeRemainingInGame);
			LOG("car location: {:.7f}, {:.7f}, {:.7f}", savedReplayState.carLocation.X, savedReplayState.carLocation.Y, savedReplayState.carLocation.Z);
			LOG("car velocity: {:.7f}, {:.7f}, {:.7f}", savedReplayState.carVelocity.X, savedReplayState.carVelocity.Y, savedReplayState.carVelocity.Z);
			LOG("car angular velocity: {:.7f}, {:.7f}, {:.7f}", savedReplayState.carAngularVelocity.X, savedReplayState.carAngularVelocity.Y, savedReplayState.carAngularVelocity.Z);
			LOG("car rotation: {}, {}, {}", savedReplayState.carRotation.Pitch, savedReplayState.carRotation.Yaw, savedReplayState.carRotation.Roll);
			LOG("ball location: {:.7f}, {:.7f}, {:.7f}", savedReplayState.ballLocation.X, savedReplayState.ballLocation.Y, savedReplayState.ballLocation.Z);
			LOG("ball rotation: {}, {}, {}", savedReplayState.ballRotation.Pitch, savedReplayState.ballRotation.Yaw, savedReplayState.ballRotation.Roll);
			LOG("ball speed: {}", savedReplayState.ballSpeed);
			//LOG("has jump? : {}", savedReplayState.hasJump ? "true" : "false");

		}



		}, "printing current state", PERMISSION_ALL);
	cvarManager->setBind("K", "saveReplaySnapshot");

}
