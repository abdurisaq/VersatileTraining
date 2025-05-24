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

}



void VersatileTraining::handleExistingTrainingData(int currentShot, int totalRounds) {


     
     
     
    while (currentTrainingData.shots.size() <= currentShot) {
         

        currentTrainingData.addShot();
    }

    
    currentShotState = currentTrainingData.shots[currentShot];
     
    shotReplicationManager.currentShotRecording = currentShotState.recording;
     
    float epsilon = 0.01f;
    
    if (abs(currentShotState.goalBlocker.first.X) < epsilon && abs(currentShotState.goalBlocker.first.Z) < epsilon && abs(currentShotState.goalBlocker.second.X) < epsilon && abs(currentShotState.goalBlocker.second.Z) < epsilon) {
        currentShotState.goalAnchors = { false, false };
    }
    else {
        currentShotState.goalAnchors = { true, true };
    }
    if (currentShotState.boostAmount == 101) {
        cvarManager->executeCommand("sv_training_limitboost -1");
    }
    else {
        cvarManager->executeCommand("sv_training_limitboost " + std::to_string(currentShotState.boostAmount));
    }

}

void VersatileTraining::handleNewTrainingData(int currentShot) {
     
     

   
    currentShotState = currentTrainingData.shots[currentShot];
     
}

void VersatileTraining::handleCreateRound() {
    currentTrainingData.addShot();
}

void VersatileTraining::handleDeleteRound(TrainingEditorWrapper cw) {
    int shotToRemove = cw.GetActiveRoundNumber();
    if (shotToRemove >= 0 && shotToRemove < currentTrainingData.shots.size()) {
         

        currentTrainingData.shots.erase(currentTrainingData.shots.begin() + shotToRemove);
   
        int totalRounds = cw.GetTotalRounds();
        if (totalRounds < currentTrainingData.shots.size()) {
             
            currentTrainingData.shots.resize(totalRounds);
            currentTrainingData.numShots = totalRounds;
        }
        
    }
    else {
         
    }
}

void VersatileTraining::handleDuplicateRound(TrainingEditorWrapper cw) {
    int currentShot = cw.GetActiveRoundNumber();
    int numRounds = cw.GetTotalRounds();
     
     
    currentTrainingData.addShot(currentTrainingData.shots[currentShot]);
}

void VersatileTraining::handleStartPlayTest() {
    
    if (isInTrainingEditor()) {
         
        if (currentTrainingData.currentEditedShot < 0) {
             

            return;
        }

        if (static_cast<size_t>(currentTrainingData.currentEditedShot) >= currentTrainingData.shots.size()) {
            LOG("currentEditedShot ({}) is out of bounds or at the end (shots.size {}). Resizing shots to {}.",
                currentTrainingData.currentEditedShot,
                currentTrainingData.shots.size(),
                currentTrainingData.currentEditedShot + 1);
            currentTrainingData.shots.resize(currentTrainingData.currentEditedShot + 1);

            currentTrainingData.numShots = currentTrainingData.shots.size();
        }
        currentTrainingData.shots[currentTrainingData.currentEditedShot] = currentShotState;

    }
    playTestStarted = true;
    //trainingData[currentTrainingData.name] = currentTrainingData; //suspect line, dont know if it should be here
}

void VersatileTraining::handleStopEditing() {
    if (!currentTrainingData.customPack) return;
    lockScene = false;
     
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
         
        return;
    }

     
     
    if (currentTrainingData.currentEditedShot < 0) {
         

        return;
    }

    if (static_cast<size_t>(currentTrainingData.currentEditedShot) >= currentTrainingData.shots.size()) {
        LOG("currentEditedShot ({}) is out of bounds or at the end (shots.size {}). Resizing shots to {}.",
            currentTrainingData.currentEditedShot,
            currentTrainingData.shots.size(),
            currentTrainingData.currentEditedShot + 1);
        currentTrainingData.shots.resize(currentTrainingData.currentEditedShot + 1);

        currentTrainingData.numShots = currentTrainingData.shots.size();
    }
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
     
    editingVariances = true;
    ballBeingEdited= false;
}

void VersatileTraining::handleEditorModeEndState() {
    editingVariances = false;
    ballBeingEdited = false;
}

void VersatileTraining::handleGameEditorActorEditingEnd() {
     
    editingVariances = false;
    lockRotation = true;
    ballBeingEdited = true;
}

void VersatileTraining::handleEndPlayTest() {
    isCarRotatable = true;

    goalBlockerEligbleToBeEdited = true;

    currentShotState.recording = shotReplicationManager.currentShotRecording;
    if (currentTrainingData.currentEditedShot < 0) {
         

        return;
    }

    if (static_cast<size_t>(currentTrainingData.currentEditedShot) >= currentTrainingData.shots.size()) {
        
        currentTrainingData.shots.resize(currentTrainingData.currentEditedShot + 1);

        currentTrainingData.numShots = currentTrainingData.shots.size();
    }
    currentTrainingData.shots[currentTrainingData.currentEditedShot] = currentShotState;
    
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
        if (currentTrainingData.currentEditedShot < 0) {
             

            return;
        }

        if (static_cast<size_t>(currentTrainingData.currentEditedShot) >= currentTrainingData.shots.size()) {
            
            currentTrainingData.shots.resize(currentTrainingData.currentEditedShot + 1);

            currentTrainingData.numShots = currentTrainingData.shots.size();
        }
        int velocity = currentTrainingData.shots[currentTrainingData.currentEditedShot].startingVelocity;
        startingVelocityTranslation = unitVector * (float)velocity;
    }

    Vector loc2 = getClampChange(loc, rot);
    if (loc2.X != 0 || loc2.Y != 0 || loc2.Z != 0) {
   
        car.SetLocation(loc2);
    }

    car.SetAngularVelocity(Vector{ 0, 0, 0 }, false);
    Vector vel = car.GetVelocity();
    car.SetVelocity({ vel.X, vel.Y, 5 });// need some z velocity because for some reason the car still starts falling if its at 0


}


void VersatileTraining::handleUnfrozenCar(ActorWrapper car, Vector loc, Rotator rot) {
    if (!frozeZVal) {
        frozenZVal = loc.Z;
        frozeZVal = true;
    }

    appliedStartingVelocity = true;


}