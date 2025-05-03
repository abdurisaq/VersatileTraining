#include "pch.h"



void SnapShotManager::takeSnapShot(GameWrapper* gw) {

	ReplayState savedReplayState;

	if (isInReplay) {

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
				savedReplayState.focusPlayerBoostAmount = boost.GetCurrentBoostAmount();
				savedReplayState.boosting = boost.GetbActive();

				LOG("boost amount : {}", savedReplayState.focusPlayerBoostAmount);
				LOG("boosting? : {}", boost.GetbActive() ? "true" : "false");

				JumpComponentWrapper jump = car.GetJumpComponent();

				if (jump.IsNull()) {
					LOG("Jump component is null");
				}


				savedReplayState.jumpTimer = !car.GetbJumped() || car.GetJumpComponent().GetInactiveTime();
				savedReplayState.hasJump = jump.GetbActive(); // dodge timer at a minimum is 1.25 seconds and a max of 1.45 if you held jump. going to be conservative and use 1.25

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

		savedReplayState.ballAngularVelocity = ball.GetAngularVelocity();
		savedReplayState.ballLocation = ball.GetLocation();
		savedReplayState.ballVelocity = ball.GetVelocity();
		savedReplayState.ballRotation = ball.GetRotation();

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
		int secondsRemaining = static_cast<int>(gw->GetCurrentGameState().GetSecondsRemaining());
		int minutes = secondsRemaining / 60;
		int seconds = secondsRemaining % 60;
		oss1 << std::setw(2) << std::setfill('0') << minutes
			<< ":" << std::setw(2) << std::setfill('0') << seconds;
		savedReplayState.timeRemainingInGame = oss1.str();
		LOG("saved replay name : {}", savedReplayState.replayName);
		LOG("saved replay player name : {}", savedReplayState.focusPlayerName);
		LOG("saved replay player boost amount : {}", savedReplayState.focusPlayerBoostAmount);
		LOG("saved replay date : {}", savedReplayState.replayTime);
		LOG("saved replay time stamp : {}", savedReplayState.formattedTimeStampOfSaved);
		LOG("saved replay time remaining : {}", savedReplayState.timeRemainingInGame);
		LOG("car location: {:.7f}, {:.7f}, {:.7f}", savedReplayState.carLocation.X, savedReplayState.carLocation.Y, savedReplayState.carLocation.Z);
		LOG("car velocity: {:.7f}, {:.7f}, {:.7f}", savedReplayState.carVelocity.X, savedReplayState.carVelocity.Y, savedReplayState.carVelocity.Z);
		LOG("car angular velocity: {:.7f}, {:.7f}, {:.7f}", savedReplayState.carAngularVelocity.X, savedReplayState.carAngularVelocity.Y, savedReplayState.carAngularVelocity.Z);
		LOG("car rotation: {}, {}, {}", savedReplayState.carRotation.Pitch, savedReplayState.carRotation.Yaw, savedReplayState.carRotation.Roll);
		LOG("ball location: {:.7f}, {:.7f}, {:.7f}", savedReplayState.ballLocation.X, savedReplayState.ballLocation.Y, savedReplayState.ballLocation.Z);
		LOG("ball velocity: {:.7f}, {:.7f}, {:.7f}", savedReplayState.ballVelocity.X, savedReplayState.ballVelocity.Y, savedReplayState.ballVelocity.Z);
		LOG("ball angular velocity: {:.7f}, {:.7f}, {:.7f}", savedReplayState.ballAngularVelocity.X, savedReplayState.ballAngularVelocity.Y, savedReplayState.ballAngularVelocity.Z);
		LOG("ball rotation: {}, {}, {}", savedReplayState.ballRotation.Pitch, savedReplayState.ballRotation.Yaw, savedReplayState.ballRotation.Roll);
		LOG("has jump? : {}", savedReplayState.hasJump ? "true" : "false");

		savedReplayState.filled = true;
	}
}