#pragma once
#include <pch.h>


struct ReplayState {

	std::string replayName;

	std::string formattedTimeStampOfSaved;
	std::string replayTime;
	std::string timeRemainingInGame;

	std::string focusPlayerName;


	Vector carVelocity;
	Vector carAngularVelocity;
	Vector carLocation;
	Rotator carRotation;

	float focusPlayerBoostAmount;
	float jumpTimer;
	bool hasJump;
	bool boosting;

	Vector ballVelocity;
	Vector ballLocation;
	Rotator ballRotation;
	Vector ballAngularVelocity;

	bool filled = false;
	std::pair<Rotator, float> getBallShotFromVelocity() const {
		float vx = ballVelocity.X;
		float vy = ballVelocity.Y;
		float vz = ballVelocity.Z;

		float speed = sqrtf(vx * vx + vy * vy + vz * vz);

		if (speed == 0.0f) {
			return { Rotator{0, 0, 0}, 0.0f };
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

		Rotator rot{ pitch_units, yaw_units, 0 };
		float clampedSpeed = min(speed, 6000.0f);

		
		return { rot, clampedSpeed };

	}

};


class SnapShotManager {


public:

	std::vector<ReplayState> replayStates;
	bool isInReplay;
	std::string focusCarID;



	void takeSnapShot(GameWrapper* gw);
};