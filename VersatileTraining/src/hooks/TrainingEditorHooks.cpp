#include "pch.h"
#include "src/core/versatileTraining.h"


void VersatileTraining::setupTrainingEditorHooks() {
    gameWrapper->HookEventWithCallerPost<ActorWrapper>(
        "Function TAGame.TrainingEditorMetrics_TA.TrainingEditorEnter",
        [this](ActorWrapper cw, void* params, std::string eventName) {
            handleTrainingEditorEnter();
        });

    gameWrapper->HookEventWithCaller<ActorWrapper>(
        "Function TAGame.GameEvent_TrainingEditor_TA.LoadRound",
        [this](ActorWrapper cw, void* params, std::string eventName) {
            handleLoadRound(cw, params, eventName);
        });

    gameWrapper->HookEventWithCaller<ActorWrapper>(
        "Function TAGame.GameMetrics_TA.EndRound",
        [this](ActorWrapper cw, void* params, std::string eventName) {
            
            if (!(isInTrainingEditor() || isInTrainingPack())) return;
            shotReplicationManager.stopRecordingShot();
            auto server = gameWrapper->GetCurrentGameState();

            if (!server) return;

            auto cars = server.GetCars();
            for (int i = 0; i < cars.Count(); ++i) {
                auto car = cars.Get(i);
                if (!car) continue;
                LOG("car name : {}", car.GetOwnerName());
                if (car.GetOwnerName() == "testplayers") {
                    LOG("destroying car");
                    auto controller = car.GetAIController();
                    car.Destroy();
                    server.RemovePlayer(controller);
                }

            }

        });

    gameWrapper->HookEventWithCallerPost<ActorWrapper>(
        "Function TAGame.TrainingEditorMetrics_TA.TrainingEditorExit",
        [this](ActorWrapper cw, void* params, std::string eventName) {
            handleTrainingEditorExit();
        });
    gameWrapper->HookEventWithCallerPost<ActorWrapper>(
		"Function GameEvent_TrainingEditor_TA.Countdown.EndState",
		[this](ActorWrapper cw, void* params, std::string eventName) {
            snapshotManager.currentReplayState.captureSource = CaptureSource::Training;
		});

    gameWrapper->HookEventWithCallerPost<ActorWrapper>(
        "Function TAGame.GameEvent_TrainingEditor_TA.Save",
        [this](ActorWrapper cw, void* params, std::string eventName) {
            handleTrainingSave();
        });

    gameWrapper->HookEventWithCallerPost<ActorWrapper>(
        "Function GameEvent_TrainingEditor_TA.ShotSelection.StartEditing",
        [this](ActorWrapper cw, void* params, std::string eventName) {
            handleStartEditing(cw);
        });
}




void VersatileTraining::handleTrainingEditorEnter() {
    LOG("Training editor enter");
    currentTrainingData.currentEditedShot = -1;
    trainingData = storageManager.loadCompressedTrainingData(storageManager.saveTrainingFilePath);
    for (auto& [key, value] : trainingData) {
        shiftToNegative(value);
    }

}

void VersatileTraining::handleLoadRound(ActorWrapper cw, void* params, std::string eventName) {
    if (!(isInTrainingEditor() || isInTrainingPack()))return;

    VersatileTraining::getTrainingData(cw, params, eventName);
    
    shotReplicationManager.canSpawnBot = true;
    LOG("setting can spawn bot to true");
    
    freezeForShot = currentShotState.freezeCar;
    frozeZVal = !currentShotState.freezeCar;
    lockRotation = true;
    appliedStartingVelocity = false;
    editingVariances = false;
    appliedWallClamping = false;
    editingGoalBlocker = false;
    appliedJumpState= false;
}

void VersatileTraining::handleTrainingEditorExit() {
    LOG("reset is benig called");
    currentTrainingData.reset();
    editingGoalBlocker = false;
    goalBlockerEligbleToBeEdited = false;
    currentShotState.goalAnchors = { false, false };

}

void VersatileTraining::handleTrainingSave() {
    if (currentTrainingData.currentEditedShot != -1) {
  
        currentTrainingData.shots[currentTrainingData.currentEditedShot] = currentShotState;
        trainingData[currentPackKey] = currentTrainingData;

        LOG("saving num shots: {}", currentTrainingData.numShots);

        for (auto& [key, value] : trainingData) {
            shiftToPositive(value);
        }
        storageManager.saveCompressedTrainingData(trainingData, storageManager.saveTrainingFilePath);
        trainingData = storageManager.loadCompressedTrainingData(storageManager.saveTrainingFilePath);
        for (auto& [key, value] : trainingData) {
            shiftToNegative(value);
        }
    }
}

void VersatileTraining::handleStartEditing(ActorWrapper cw) {
    LOG("new training pack opened and editing started--------------");
    isCarRotatable = true;
    goalBlockerEligbleToBeEdited = true;

    if (!cw) {
        LOG("Caller is invalid");
        return;
    }

    TrainingEditorWrapper tw(cw.memory_address);
    if (tw.IsNull()) {
        LOG("Failed to get TrainingEditorWrapper");
        return;
    }

    GameEditorSaveDataWrapper data = tw.GetTrainingData();
    TrainingEditorSaveDataWrapper td = data.GetTrainingData();

    std::string name = td.GetTM_Name().ToString();
    int currentShot = tw.GetActiveRoundNumber();
    int totalRounds = tw.GetTotalRounds();

    LOG("Training pack name: {}", name);
    LOG("Training current shot: {}", currentShot);
    LOG("Training num rounds: {}", totalRounds);

    if (currentTrainingData.currentEditedShot != -1) {
        currentTrainingData.currentEditedShot = currentShot;
        LOG("already loaded, skipping searching training data, current shot: {}, total rounds: {}", currentTrainingData.currentEditedShot, totalRounds);
        handleExistingTrainingData(currentShot, totalRounds);
        return;
    }

    bool found = false;
    for (auto& [key, value] : trainingData) {
        if (value.name == name) {
            currentTrainingData = value;
            LOG("Training pack found in trainingData");
            found = true;
            currentPackKey = key;
            break;
        }
    }

    if (!found) {
        LOG("Training pack not found in trainingData");
        currentTrainingData.initCustomTrainingData(totalRounds, name);
        auto [it, inserted] = trainingData.insert_or_assign(name, currentTrainingData);
        currentPackKey = name;
    }

    unlockStartingVelocity = false;
    handleNewTrainingData(currentShot);
}


void VersatileTraining::getTrainingData(ActorWrapper cw, void* params, std::string eventName) {


    LOG("getTrainingData called, looking for custom training pack");
    auto tw = ((TrainingEditorWrapper)cw.memory_address);
    GameEditorSaveDataWrapper data = tw.GetTrainingData();
    TrainingEditorSaveDataWrapper td = data.GetTrainingData();
    LOG("Training pack code: {}", td.GetCode().ToString());
    std::string name = td.GetTM_Name().ToString();
    
    LOG("Training pack name", name);
    /*GetCreatorName();
    GetDescription();*/
    LOG("Training pack type: {}", td.GetType());
    LOG("Training pack difficulty: {}", td.GetDifficulty());
    LOG("Training pack creator name: {}", td.GetCreatorName().ToString());
    LOG("Training pack description: {}", td.GetDescription().ToString());
    LOG("training pack created at: {}", td.GetCreatedAt());
    int totalRounds = tw.GetTotalRounds();
    LOG("Training num rounds: {}", totalRounds);

    int currentShot = tw.GetActiveRoundNumber();
    LOG("Training current shot : {}", currentShot);

    snapshotManager.currentReplayState.replayName = name + " Shot: " + std::to_string(currentShot); //training pack name
    snapshotManager.currentReplayState.captureSource = CaptureSource::Training;

    std::string code = td.GetCode().ToString();
    
    bool found = false;
    for (auto& [key, value] : trainingData) {
        if (value.name == name) {
            if (currentTrainingData.name != value.name) {
                currentTrainingData = value;
            }
            LOG("num shots in found training pack: {}", currentTrainingData.numShots);
            if (totalRounds < currentTrainingData.numShots) {
                LOG("resizing because num shots in training pack is less than the saved version");
                currentTrainingData.shots.resize(totalRounds);
            }
            currentTrainingData.customPack = true;
            currentTrainingData.currentEditedShot = currentShot;
            currentShotState = currentTrainingData.shots[currentShot];
            LOG("setting active boost amount to {}", currentShotState.boostAmount);
            LOG("setting active starting velocity to {}", currentShotState.startingVelocity);
            LOG("pulled goalblocker, x1:{}, z1:{} x2:{} z2{}. setting anchor first to : {}, and send to : {}", currentShotState.goalBlocker.first.X, currentShotState.goalBlocker.first.Z, currentShotState.goalBlocker.second.X, currentShotState.goalBlocker.second.Z, currentShotState.goalAnchors.first ? "true" : "false", currentShotState.goalAnchors.second ? "true" : "false");

            if (currentShotState.freezeCar) {
                LOG("car is frozen");
            }
            else {
                LOG("car is not frozen");
            }
           
            if (currentShotState.boostAmount == 101) {
                cvarManager->executeCommand("sv_training_limitboost -1");
            }
            else {
                cvarManager->executeCommand("sv_training_limitboost " + std::to_string(currentShotState.boostAmount));
            }
            
            found = true;
        }
    }
    if (!found) {
        LOG("didn't find this traini pack in training data");
        if (currentTrainingData.name == name) {
            
        }
        else {
            currentTrainingData.initCustomTrainingData(totalRounds, name);
        }
        currentShotState = currentTrainingData.shots[currentShot];
        
        if (td.GetCode().ToString().empty()) {
            currentTrainingData.customPack = true;
        }
        else {
            currentTrainingData.customPack = false;
        }
        
        cvarManager->executeCommand("sv_training_limitboost -1");
       
    }


}