#pragma once
#include "pch.h"

struct ShotRecording {
    int carBody = 23; // default octane
    GamepadSettings settings = GamepadSettings(0, 0.5, 1, 1);
    std::vector<ControllerInput> inputs;
    RBState startState;
    struct InitialState {
        Vector location;
        Rotator rotation;
        Vector velocity;
    };

    std::shared_ptr<InitialState> initialState;
};

class ShotReplicationManager {
public:
	bool canStartPlayback = false;


	bool startPlayback = false;
	bool testCalledInStartRound = false;

	bool botSpawnedTest = false;
	bool canSpawnBot = false;
	bool primedToStartRecording = false;
	bool roundStarted = false;

	bool startRecording = false;
	bool recording = false;
	int frame = 0;
	std::shared_ptr< ShotRecording> currentShotRecording;
	
	void startRecordingShot(GameWrapper* gw);
	void spawnBot(GameWrapper* gw);
	void stopRecordingShot();
	
};

//haven't made yet, but need to add code here for input compression to a bitstream, then uncompressing