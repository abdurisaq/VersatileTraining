#include "pch.h"



void SnapShotManager::takeSnapShot(GameWrapper* gw) {

	ReplayState currentReplayState;

	if (currentReplayState.capturedFromReplay) {

		ReplayServerWrapper serverReplay = gw->GetGameEventAsReplay();
		if (serverReplay.IsNull()) {
			LOG("Server replay is null1");
			return;
		}
		ReplayWrapper replay = serverReplay.GetReplay();
		if (replay.IsNull()) {
			LOG("Replay is null2");
			return;
		}
		currentReplayState.replayName = replay.GetReplayName().ToString();

		currentReplayState.replayTime = replay.GetDate().ToString();

		ArrayWrapper<CarWrapper> cars = serverReplay.GetCars();
		bool found = false;
		for (auto car : cars) {
			if (car.GetPRI().GetUniqueIdWrapper().str() == focusCarID) {
				LOG("car location: {:.7f}, {:.7f}, {:.7f}", car.GetLocation().X, car.GetLocation().Y, car.GetLocation().Z);
				currentReplayState.carLocation = car.GetLocation();
				currentReplayState.carVelocity = car.GetVelocity();
				currentReplayState.carAngularVelocity = car.GetAngularVelocity();
				currentReplayState.carRotation = car.GetRotation();
				currentReplayState.focusPlayerName = car.GetOwnerName();

				BoostWrapper boost = car.GetBoostComponent();
				if (boost.IsNull()) {
					LOG("Boost component is null");
					return;
				}
				currentReplayState.focusPlayerBoostAmount = boost.GetCurrentBoostAmount();
				currentReplayState.boosting = boost.GetbActive();

				LOG("boost amount : {}", currentReplayState.focusPlayerBoostAmount);
				LOG("boosting? : {}", boost.GetbActive() ? "true" : "false");

				JumpComponentWrapper jump = car.GetJumpComponent();

				if (jump.IsNull()) {
					LOG("Jump component is null");
				}


				currentReplayState.jumpTimer = !car.GetbJumped() || car.GetJumpComponent().GetInactiveTime();
				currentReplayState.hasJump = jump.GetbActive(); // dodge timer at a minimum is 1.25 seconds and a max of 1.45 if you held jump. going to be conservative and use 1.25

				LOG("jump inactive timer : {}", car.GetJumpComponent().GetInactiveTime());
				LOG("double jumped? : {}", car.GetbDoubleJumped() ? "true" : "false");
				LOG("jumped? : {}", currentReplayState.hasJump ? "true" : "false");
				found = true;
				break;
			}
		}

		BallWrapper ball = serverReplay.GetBall();
		if (ball.IsNull()) {
			LOG("Ball is null");
			return;
		}

		currentReplayState.ballAngularVelocity = ball.GetAngularVelocity();
		currentReplayState.ballLocation = ball.GetLocation();
		currentReplayState.ballVelocity = ball.GetVelocity();
		currentReplayState.ballRotation = ball.GetRotation();

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
		currentReplayState.formattedTimeStampOfSaved = oss.str();
		std::ostringstream oss1;
		int secondsRemaining = static_cast<int>(gw->GetCurrentGameState().GetSecondsRemaining());
		int minutes = secondsRemaining / 60;
		int seconds = secondsRemaining % 60;
		oss1 << std::setw(2) << std::setfill('0') << minutes
			<< ":" << std::setw(2) << std::setfill('0') << seconds;
		currentReplayState.timeRemainingInGame = oss1.str();
		LOG("saved replay name : {}", currentReplayState.replayName);
		LOG("saved replay player name : {}", currentReplayState.focusPlayerName);
		LOG("saved replay player boost amount : {}", currentReplayState.focusPlayerBoostAmount);
		LOG("saved replay date : {}", currentReplayState.replayTime);
		LOG("saved replay time stamp : {}", currentReplayState.formattedTimeStampOfSaved);
		LOG("saved replay time remaining : {}", currentReplayState.timeRemainingInGame);
		LOG("car location: {:.7f}, {:.7f}, {:.7f}", currentReplayState.carLocation.X, currentReplayState.carLocation.Y, currentReplayState.carLocation.Z);
		LOG("car velocity: {:.7f}, {:.7f}, {:.7f}", currentReplayState.carVelocity.X, currentReplayState.carVelocity.Y, currentReplayState.carVelocity.Z);
		LOG("car angular velocity: {:.7f}, {:.7f}, {:.7f}", currentReplayState.carAngularVelocity.X, currentReplayState.carAngularVelocity.Y, currentReplayState.carAngularVelocity.Z);
		LOG("car rotation: {}, {}, {}", currentReplayState.carRotation.Pitch, currentReplayState.carRotation.Yaw, currentReplayState.carRotation.Roll);
		LOG("ball location: {:.7f}, {:.7f}, {:.7f}", currentReplayState.ballLocation.X, currentReplayState.ballLocation.Y, currentReplayState.ballLocation.Z);
		LOG("ball velocity: {:.7f}, {:.7f}, {:.7f}", currentReplayState.ballVelocity.X, currentReplayState.ballVelocity.Y, currentReplayState.ballVelocity.Z);
		LOG("ball angular velocity: {:.7f}, {:.7f}, {:.7f}", currentReplayState.ballAngularVelocity.X, currentReplayState.ballAngularVelocity.Y, currentReplayState.ballAngularVelocity.Z);
		LOG("ball rotation: {}, {}, {}", currentReplayState.ballRotation.Pitch, currentReplayState.ballRotation.Yaw, currentReplayState.ballRotation.Roll);
		LOG("has jump? : {}", currentReplayState.hasJump ? "true" : "false");

		currentReplayState.filled = true;
	}
	else if (currentReplayState.capturedFromTraining) {


	}
	else {
		LOG("Not in replay or training mode, unable to take snapshot");
		return;
	}

	replayStates.push_back(currentReplayState);
}