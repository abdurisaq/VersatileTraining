#include "pch.h"
#include "src/core/versatileTraining.h"


void VersatileTraining::setupTrainingShotHooks() {
    gameWrapper->HookEventWithCallerPost<TrainingEditorWrapper>(
        "Function TAGame.GFxData_TrainingModeEditor_TA.CreateRound",
        [this](TrainingEditorWrapper cw, void* params, std::string eventName) {
            handleCreateRound();
        });

    gameWrapper->HookEventWithCallerPost<TrainingEditorWrapper>(
        "Function TAGame.GameEvent_TrainingEditor_TA.DeleteRound",
        [this](TrainingEditorWrapper cw, void* params, std::string eventName) {
            handleDeleteRound(cw);
        });

    gameWrapper->HookEventWithCallerPost<TrainingEditorWrapper>(
        "Function GameEvent_TrainingEditor_TA.ShotSelection.DuplicateRound",
        [this](TrainingEditorWrapper cw, void* params, std::string eventName) {
            handleDuplicateRound(cw);
        });

    gameWrapper->HookEventWithCallerPost<TrainingEditorWrapper>(
        "Function TAGame.GameEvent_TrainingEditor_TA.StartPlayTest",
        [this](TrainingEditorWrapper cw, void* params, std::string eventName) {
            handleStartPlayTest();
        });
    gameWrapper->HookEvent(
		"Function TAGame.GameEvent_TrainingEditor_TA.EndPlayTest",
        [this](std::string eventName) {
            handleEndPlayTest();
			playTestStarted = false;
		});

    gameWrapper->HookEventWithCallerPost<TrainingEditorWrapper>(
        "Function GameEvent_TrainingEditor_TA.EditorMode.StopEditing",
        [this](TrainingEditorWrapper cw, void* params, std::string eventName) {
            handleStopEditing();
        });

    gameWrapper->HookEventWithCallerPost<TrainingEditorWrapper>(
        "Function TAGame.Ball_GameEditor_TA.EditingEnd",
        [this](TrainingEditorWrapper cw, void* params, std::string eventName) {
            handleBallEditingEnd();
        });

    gameWrapper->HookEventWithCallerPost<TrainingEditorWrapper>(
        "Function GameEvent_GameEditor_TA.EditorMode.EndState",
        [this](TrainingEditorWrapper cw, void* params, std::string eventName) {
            handleEditorModeEndState();
        });

    gameWrapper->HookEventWithCallerPost<TrainingEditorWrapper>(
        "Function TAGame.GameEditor_Actor_TA.EditingEnd",
        [this](TrainingEditorWrapper cw, void* params, std::string eventName) {
            handleGameEditorActorEditingEnd();
        });

    gameWrapper->HookEvent(
        "Function TAGame.GameEvent_TrainingEditor_TA.EndPlayTest",
        [this](std::string eventName) {
            handleEndPlayTest();
        });
}



void VersatileTraining::handleExistingTrainingData(int currentShot, int totalRounds) {


    LOG("already loaded, skipping searching training data");
    LOG("currentShot: {}", currentTrainingData.currentEditedShot);
    LOG("amount of shots in found training data existing: {}", currentTrainingData.shots.size());

    /*while (currentShot >= currentTrainingData.shots.size()) {
        LOG("resizing");
        currentTrainingData.shots.resize(currentShot + 1);
        currentTrainingData.numShots = currentShot + 1;
        LOG("now this many shots in training data: {}", currentTrainingData.numShots);
    }*/

    currentShotState = currentTrainingData.shots[currentShot];
    shotReplicationManager.currentShotRecording = currentShotState.recording;
    LOG("pulled goalblocker, x1:{}, z1:{} x2:{} z2{}. setting anchor first to : {}, and send to : {}", currentShotState.goalBlocker.first.X, currentShotState.goalBlocker.first.Z, currentShotState.goalBlocker.second.X, currentShotState.goalBlocker.second.Z, currentShotState.goalAnchors.first ? "true" : "false", currentShotState.goalAnchors.second ? "true" : "false");
    float epsilon = 0.01f;

    if (abs(currentShotState.goalBlocker.first.X) < epsilon && abs(currentShotState.goalBlocker.first.Z) < epsilon && abs(currentShotState.goalBlocker.second.X) < epsilon && abs(currentShotState.goalBlocker.second.Z) < epsilon) {
        currentShotState.goalAnchors = { false, false };
    }
    else {
        currentShotState.goalAnchors = { true, true };
    }
}

void VersatileTraining::handleNewTrainingData(int currentShot) {
    LOG("currentShot: {}", currentShot);
    LOG("amount of shots in found training data new : {}", currentTrainingData.shots.size());

    /*while (currentShot >= currentTrainingData.shots.size()) {
        LOG("resizing");
        currentTrainingData.shots.resize(currentShot + 1);
        currentTrainingData.numShots = currentShot + 1;
        LOG("now this many shots in training data: {}", currentTrainingData.numShots);
    }*/

    currentShotState = currentTrainingData.shots[currentShot];
    LOG("pulled goalblocker, x1:{}, z1:{} x2:{} z2{}. setting anchor first to : {}, and send to : {}", currentShotState.goalBlocker.first.X, currentShotState.goalBlocker.first.Z, currentShotState.goalBlocker.second.X, currentShotState.goalBlocker.second.Z, currentShotState.goalAnchors.first ? "true" : "false", currentShotState.goalAnchors.second ? "true" : "false");
}

void VersatileTraining::handleCreateRound() {
    currentTrainingData.addShot();
}

void VersatileTraining::handleDeleteRound(TrainingEditorWrapper cw) {
    int shotToRemove = cw.GetActiveRoundNumber();
    if (shotToRemove >= 0 && shotToRemove < currentTrainingData.numShots) {
        LOG("Removing shot: {}", shotToRemove);

        currentTrainingData.shots.erase(currentTrainingData.shots.begin() + shotToRemove);
        currentTrainingData.numShots--;
        int totalRounds = cw.GetTotalRounds();
        if (totalRounds < currentTrainingData.numShots) {
            LOG("resizing");
            currentTrainingData.shots.resize(totalRounds);
            currentTrainingData.numShots = totalRounds;
        }
        //trainingData[currentTrainingData.name] = currentTrainingData;
    }
    else {
        LOG("Invalid shot index: {}, numShots = {}", shotToRemove, currentTrainingData.numShots);
    }
}

void VersatileTraining::handleDuplicateRound(TrainingEditorWrapper cw) {
    int currentShot = cw.GetActiveRoundNumber();
    int numRounds = cw.GetTotalRounds();
    LOG("duplicating shot {}", currentShot);
    LOG("num rounds {}", numRounds);
    currentTrainingData.addShot(currentTrainingData.shots[currentShot]);
}

void VersatileTraining::handleStartPlayTest() {
    
    if (isInTrainingEditor()) {
        LOG("changing shot state ");
        currentTrainingData.shots[currentTrainingData.currentEditedShot] = currentShotState;

    }
    playTestStarted = true;
    //trainingData[currentTrainingData.name] = currentTrainingData; //suspect line, dont know if it should be here
}

void VersatileTraining::handleStopEditing() {
    if (!currentTrainingData.customPack) return;
    lockScene = false;
    LOG("stopped editing");
    isCarRotatable = false;
    lockRotation = true;
    editingVariances = false;
    rotationToApply = { 0, 0, 0 };
    if (goalBlockerEligbleToBeEdited) {
        goalBlockerEligbleToBeEdited = false;
        editingGoalBlocker = false;
        rectangleSaved = false;
    }

    if (currentTrainingData.currentEditedShot == -1) {
        LOG("currentTrainingDataUsed is null");
        return;
    }

    LOG("saving to shot in training data {}", currentTrainingData.currentEditedShot);
    LOG("temp boost amount: {}", currentShotState.boostAmount);

    currentTrainingData.shots[currentTrainingData.currentEditedShot] = currentShotState;
    currentShotState.goalBlocker = { { 0, 5140, 0 }, { 0, 5140, 0 } };
    currentShotState.goalAnchors = { false, false };
    //trainingData[currentTrainingData.name] = currentTrainingData;

    LOG("adding shot training data: {}, boost amount: {}, starting velocity: {}",
        currentTrainingData.currentEditedShot,
        currentTrainingData.shots[currentTrainingData.currentEditedShot].boostAmount,
        currentTrainingData.shots[currentTrainingData.currentEditedShot].startingVelocity);
}

void VersatileTraining::handleBallEditingEnd() {
    LOG("car is being edited");
    editingVariances = true;
    ballBeingEdited= false;
}

void VersatileTraining::handleEditorModeEndState() {
    editingVariances = false;
    ballBeingEdited = false;
}

void VersatileTraining::handleGameEditorActorEditingEnd() {
    LOG("ball is being edited");
    editingVariances = false;
    lockRotation = true;
    ballBeingEdited = true;
}

void VersatileTraining::handleEndPlayTest() {
    isCarRotatable = true;

    LOG("handleEndPlayTest - recording inputs size: {}",
        shotReplicationManager.currentShotRecording.inputs.size());
    LOG("handleEndPlayTest - carBody: {}",
        shotReplicationManager.currentShotRecording.carBody);

    currentShotState.recording = shotReplicationManager.currentShotRecording;
    currentTrainingData.shots[currentTrainingData.currentEditedShot] = currentShotState;
    LOG("After assignment - recording inputs size: {}",
        currentShotState.recording.inputs.size());

}



void VersatileTraining::handleFreezeCar(CarWrapper car, Vector loc, Rotator rot) {
    if (rot.Pitch != carRotationUsed.Pitch || rot.Yaw != carRotationUsed.Yaw) {


        if (!frozeZVal) {
            frozenZVal = loc.Z;
            frozeZVal = true;
        }


        float pitchRad = (float)((rot.Pitch / 16201.0f) * (PI / 2));
        float yawRad = (float)((rot.Yaw / 32768.0f) * PI);
        float z = sinf(pitchRad);
        float y = cosf(pitchRad) * sinf(yawRad);
        float x = cosf(pitchRad) * cosf(yawRad);
        Vector unitVector = { x, y, z };
        int velocity = currentTrainingData.shots[currentTrainingData.currentEditedShot].startingVelocity;
        startingVelocityTranslation = unitVector * (float)velocity;
    }

    Vector loc2 = getClampChange(loc, rot);
    if (loc2.X != 0 || loc2.Y != 0 || loc2.Z != 0) {
        /*LOG("old location - X: {}, Y: {}, Z: {}", loc.X, loc.Y, loc.Z);
        LOG("new location - X: {}, Y: {}, Z: {}", loc2.X, loc2.Y, loc2.Z);*/
        car.SetLocation(loc2);
    }

    car.SetAngularVelocity(Vector{ 0, 0, 0 }, false);
    Vector vel = car.GetVelocity();
    car.SetVelocity({ vel.X, vel.Y, 5 });//5


}


void VersatileTraining::handleUnfrozenCar(ActorWrapper car, Vector loc, Rotator rot) {
    if (!frozeZVal) {
        frozenZVal = loc.Z;
        frozeZVal = true;
    }

    appliedStartingVelocity = true;


}