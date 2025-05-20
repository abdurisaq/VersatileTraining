#pragma once
#include <pch.h>

struct SnapshotGroup {
    std::string uid; 
    std::string name;
    std::vector<size_t> snapshotOriginalIndices;
    
};

enum class CaptureSource {
	Replay,
	Training,
	Unknown
};

struct ReplayState {

	std::string replayName;

	std::string formattedTimeStampOfSaved;
	
	//replay specific fields
	std::string replayTime;
	std::string timeRemainingInGame;
	std::string focusPlayerName;

	CaptureSource captureSource = CaptureSource::Unknown;

	Vector carVelocity;
	Vector carAngularVelocity;
	Vector carLocation;
	Rotator carRotation;

	int boostAmount;
	float jumpTimer;
	bool hasJump;
	bool boosting;

	Vector ballLocation;
	float ballSpeed;
	Rotator ballRotation;

	bool ballSet = false;
	bool carLocationSet = false;
	bool carRotationSet = false;

	bool empty = true;
	void setBallStartingRotationAndStrength(Rotator rot, float strength);
	void setBallStartingRotationAndStrength(Vector velocity);

	ReplayState(): replayName(""), formattedTimeStampOfSaved(""), replayTime(""), timeRemainingInGame(""), focusPlayerName(""),
		captureSource(CaptureSource::Unknown), carVelocity({0,0,0}), carAngularVelocity({0,0,0}), carLocation({0,0,0}), carRotation({0,0,0}),
		boostAmount(0), jumpTimer(0), hasJump(false), boosting(false), ballLocation({ 0, 0, 0 }), ballSpeed(0), ballRotation({ 0, 0, 0 }), ballSet(true), carLocationSet(true),carRotationSet(true),empty(true) {}
	

};


class SnapShotManager {


public:

	std::vector<ReplayState> replayStates;
	bool isInReplay;
	std::string focusCarID;

	ReplayState currentReplayState;

	void takeSnapShot(GameWrapper* gw,std::string focusID);
};