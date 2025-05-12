#include "pch.h"
#include "src/core/versatileTraining.h"


void VersatileTraining::setupTrainingEditorHooks() {
    gameWrapper->HookEventWithCallerPost<ActorWrapper>("Function TAGame.GFxData_TrainingMode_TA.HandleAddTrainingData",
        [this](ActorWrapper caller, void* params, std::string eventName)
        {
            struct cRPC_HandleAddTrainingData {
                unsigned char pad[0x00E8];
                uintptr_t TrainingData;
            };
            struct pHandleAddTrainingData {
                cRPC_HandleAddTrainingData* RPC;
            };
            pHandleAddTrainingData* p = (pHandleAddTrainingData*)params;
            TrainingEditorSaveDataWrapper data = TrainingEditorSaveDataWrapper(p->RPC->TrainingData);
            std::string code = data.GetCode().ToString();
            std::string name = data.GetTM_Name().ToString();

            LOG("Code: {}", code);

            std::string oldKey = "";
            bool found = false;

            for (auto& [key, value] : trainingData) {
                if (value.name == name) {
                    oldKey = key;
                    value.code = code;
                    found = true;
                    LOG("found training pack with name: {}", value.name);
                    LOG("code: {}", code);
                    break;
                }
            }

            if (found && !code.empty() && oldKey != code) {
                LOG("Reorganizing pack - old key: {}, new key: {}", oldKey, code);
                CustomTrainingData packData = trainingData[oldKey];
                trainingData.erase(oldKey);
                trainingData[code] = packData;
                std::filesystem::path packFolder = myDataFolder / "TrainingPacks" / storageManager.recordingStorage.sanitizeFilename(packData.name);

                if (!std::filesystem::exists(packFolder)) {
                    LOG("Training pack folder not found: {}", packFolder.string());
                    return false;
                }

                try {
                    LOG("Deleting training pack folder: {}", packFolder.string());
                    std::size_t removedCount = std::filesystem::remove_all(packFolder);
                    LOG("Removed {} files/directories", removedCount);
                    return true;
                }
                catch (const std::filesystem::filesystem_error& e) {
                    LOG("Error deleting training pack folder: {}", e.what());
                    return false;
                }
                trainingData[code].code = code;
                if (currentPackKey == oldKey) {
                    currentPackKey = code;
                }
            }
        });

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
            if (shotReplicationManager.startRecording) {
                currentShotState.recording = shotReplicationManager.currentShotRecording;
                currentTrainingData.shots[currentTrainingData.currentEditedShot] = currentShotState;
                if (shotReplicationManager.startRecording && 
                    !currentTrainingData.name.empty() && 
                    currentTrainingData.currentEditedShot >= 0) {
                    
                    auto it = trainingData.find(currentTrainingData.name);
                    if (it != trainingData.end()) {
                        // Then check if the shot index is valid
                        if (currentTrainingData.currentEditedShot < it->second.shots.size()) {
                            // Now it's safe to update the recording
                            currentShotState.recording = shotReplicationManager.currentShotRecording;
                            it->second.shots[currentTrainingData.currentEditedShot].recording = currentShotState.recording;
                            LOG("Updated recording for pack '{}', shot {}, with {} inputs", 
                                currentTrainingData.name, 
                                currentTrainingData.currentEditedShot,
                                currentShotState.recording.inputs.size());
                        } else {
                            LOG("Shot index {} is out of bounds for pack '{}'", 
                                currentTrainingData.currentEditedShot, 
                                currentTrainingData.name);
                        }
                    } else {
                        LOG("Training pack '{}' not found in training data map", 
                            currentTrainingData.name);

                    }
                }
            }
            
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
    //trainingData = storageManager.loadCompressedTrainingData(storageManager.saveTrainingFilePath);
    /*trainingData = storageManager.loadCompressedTrainingDataWithRecordings(myDataFolder);
    for (auto& [key, value] : trainingData) {
        shiftToNegative(value);
    }*/

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

    /*std::filesystem::path trainingDataPath = myDataFolder / "TrainingPacks";
    storageManager.recordingStorage.loadAllRecordings(trainingData, trainingDataPath);*/
    currentTrainingData.reset();
    playTestStarted = false;
    editingGoalBlocker = false;
    goalBlockerEligbleToBeEdited = false;
    currentShotState.goalAnchors = { false, false };

}

void VersatileTraining::handleTrainingSave() {
    if (currentTrainingData.currentEditedShot != -1) {
  
        currentTrainingData.shots[currentTrainingData.currentEditedShot] = currentShotState;

        // Validate currentPackKey - don't allow empty keys
        if (currentPackKey.empty()) {
            if (!currentTrainingData.name.empty()) {
                currentPackKey = currentTrainingData.name;
                LOG("Empty package key detected, using name instead: {}", currentPackKey);
                LOG("size of training data: {}", trainingData.size() + 1); 
                LOG("currentTrainigData size: {}", currentTrainingData.shots.size());
            }
            else {
                currentPackKey = "unnamed_pack_" + std::to_string(time(nullptr));
                LOG("Empty package key with empty name, using generated key: {}", currentPackKey);
            }
        }
        if (currentTrainingData.shots.empty()) {
            LOG("training data is empty, not saving anything" );
            return;
            //nothing to save, and dont overwrite the previous
        }
        // Now save with valid key
        trainingData[currentPackKey] = currentTrainingData;

        LOG("saving num shots: {}", currentTrainingData.numShots);

        for (auto& [key, value] : trainingData) {
            shiftToPositive(value);
        }
        //storageManager.saveCompressedTrainingData(trainingData, storageManager.saveTrainingFilePath);
        storageManager.saveCompressedTrainingDataWithRecordings(trainingData, myDataFolder);
        //trainingData = storageManager.loadCompressedTrainingData(storageManager.saveTrainingFilePath);
        //trainingData = storageManager.loadCompressedTrainingDataWithRecordings(myDataFolder);

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

    if (currentTrainingData.name == name) {
        currentTrainingData.currentEditedShot = currentShot;
        unlockStartingVelocity = false;
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

    LOG("Training pack name: {}", name);
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


    if (currentTrainingData.name == name) {
        currentTrainingData.currentEditedShot = currentShot;
        unlockStartingVelocity = false;
        LOG("already loaded, skipping searching training data, current shot: {}, total rounds: {}", currentTrainingData.currentEditedShot, totalRounds);
        handleExistingTrainingData(currentShot, totalRounds);
        return;
    }
    bool found = false;
    for (auto& [key, value] : trainingData) {
        if (value.name == name) {
            
            currentTrainingData = value;
          
            LOG("num shots in found training pack: {}", currentTrainingData.shots.size());
            if (totalRounds < currentTrainingData.numShots) {
                LOG("resizing because num shots in training pack is less than the saved version");
                currentTrainingData.shots.resize(totalRounds);
            }
            currentTrainingData.customPack = true;
            currentTrainingData.currentEditedShot = currentShot;
            LOG("currentTraining data recording size : {}", currentTrainingData.shots[currentShot].recording.inputs.size());
            currentShotState = currentTrainingData.shots[currentShot];
            shotReplicationManager.currentShotRecording = currentShotState.recording;
            LOG("shot replication current shot recording size: {}", shotReplicationManager.currentShotRecording.inputs.size());
            LOG("setting active boost amount to {}", currentShotState.boostAmount);
            LOG("setting active starting velocity to {}", currentShotState.startingVelocity);
            LOG("pulled goalblocker, x1:{}, z1:{} x2:{} z2{}. setting anchor first to : {}, and send to : {}", currentShotState.goalBlocker.first.X, currentShotState.goalBlocker.first.Z, currentShotState.goalBlocker.second.X, currentShotState.goalBlocker.second.Z, currentShotState.goalAnchors.first ? "true" : "false", currentShotState.goalAnchors.second ? "true" : "false");

            float epsilon = 0.01f; 

            if (abs(currentShotState.goalBlocker.first.X) < epsilon && abs(currentShotState.goalBlocker.first.Z) < epsilon && abs(currentShotState.goalBlocker.second.X) < epsilon && abs(currentShotState.goalBlocker.second.Z) < epsilon) {
				currentShotState.goalAnchors = { false, false };
			}
            else {
				currentShotState.goalAnchors = { true, true };
			}
           
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
            if (currentShotState.boostAmount == 101) {
                cvarManager->executeCommand("sv_training_limitboost -1");
            }
            else {
                cvarManager->executeCommand("sv_training_limitboost " + std::to_string(currentShotState.boostAmount));
            }
        }
        else {
            cvarManager->executeCommand("sv_training_limitboost -1");
            currentTrainingData.initCustomTrainingData(totalRounds, name);
        }
        currentShotState = currentTrainingData.shots[currentShot];

        if (td.GetCode().ToString().empty()) {
            currentTrainingData.customPack = true;
        }
        else {
            currentTrainingData.customPack = false;
        }

        
    }


}