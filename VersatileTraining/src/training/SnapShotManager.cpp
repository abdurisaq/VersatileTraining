#include "pch.h"



void SnapShotManager::takeSnapShot(GameWrapper* gw, std::string focusID) {

	//temporary solution
	focusCarID = focusID;
	/*ReplayState currentReplayState;*/

	if (currentReplayState.captureSource == CaptureSource::Replay) {

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
				currentReplayState.boostAmount = static_cast<int>(boost.GetCurrentBoostAmount() * 100.0f);
				currentReplayState.boosting = boost.GetbActive();

				LOG("boost amount : {}", currentReplayState.boostAmount);
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

		
		currentReplayState.ballLocation = ball.GetLocation();
		currentReplayState.setBallStartingRotationAndStrength(ball.GetVelocity());

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
		LOG("saved replay player boost amount : {}", currentReplayState.boostAmount);
		LOG("saved replay date : {}", currentReplayState.replayTime);
		LOG("saved replay time stamp : {}", currentReplayState.formattedTimeStampOfSaved);
		LOG("saved replay time remaining : {}", currentReplayState.timeRemainingInGame);
		LOG("car location: {:.7f}, {:.7f}, {:.7f}", currentReplayState.carLocation.X, currentReplayState.carLocation.Y, currentReplayState.carLocation.Z);
		LOG("car velocity: {:.7f}, {:.7f}, {:.7f}", currentReplayState.carVelocity.X, currentReplayState.carVelocity.Y, currentReplayState.carVelocity.Z);
		LOG("car angular velocity: {:.7f}, {:.7f}, {:.7f}", currentReplayState.carAngularVelocity.X, currentReplayState.carAngularVelocity.Y, currentReplayState.carAngularVelocity.Z);
		LOG("car rotation: {}, {}, {}", currentReplayState.carRotation.Pitch, currentReplayState.carRotation.Yaw, currentReplayState.carRotation.Roll);
		LOG("ball location: {:.7f}, {:.7f}, {:.7f}", currentReplayState.ballLocation.X, currentReplayState.ballLocation.Y, currentReplayState.ballLocation.Z);
		LOG("ball rotation: {}, {}, {}", currentReplayState.ballRotation.Pitch, currentReplayState.ballRotation.Yaw, currentReplayState.ballRotation.Roll);
		LOG("ball speed: {}", currentReplayState.ballSpeed);
		LOG("has jump? : {}", currentReplayState.hasJump ? "true" : "false");

		
	}
	else if (currentReplayState.captureSource == CaptureSource::Training) {
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


	}
	else {
		LOG("Not in replay or training mode, unable to take snapshot");
		return;
	}

	replayStates.push_back(currentReplayState);
}






void ReplayState::setBallStartingRotationAndStrength(Rotator rot, float strength) {
	ballRotation = rot;
	ballSpeed = strength;
}
void ReplayState::setBallStartingRotationAndStrength(Vector velocity) {
	float vx = velocity.X;
	float vy = velocity.Y;
	float vz = velocity.Z;

	float speed = sqrtf(vx * vx + vy * vy + vz * vz);

	if (speed == 0.0f) {
		ballRotation  =  Rotator(0, 0, 0);
		ballSpeed = 0.0f;
		return; 
	}

	float nx = vx / speed;
	float ny = vy / speed;
	float nz = vz / speed;

	float pitch_deg = std::asin(nz) * (180.0f / static_cast<float>(PI));
	float yaw_deg = std::atan2(ny, nx) * (180.0f / static_cast<float>(PI));


	if (yaw_deg < 0.0f) {
		yaw_deg += 360.0f;
	}

	int32_t pitch_units = static_cast<int32_t>(pitch_deg * (16384.0f / 90.0f));


	int32_t yaw_units = static_cast<int32_t>(yaw_deg * (65536.0f / 360.0f));


	pitch_units = max(-16384, min(16384, pitch_units));

	ballRotation = Rotator( pitch_units, yaw_units, 0 );
	ballSpeed = min(speed, 6000.0f);
	
}