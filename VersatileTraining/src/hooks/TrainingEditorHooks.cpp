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
            std::string code_from_game = data.GetCode().ToString();
            std::string name_from_game = data.GetTM_Name().ToString();

            LOG("HandleAddTrainingData: Game pack name: '{}', Game pack code: '{}'", name_from_game, code_from_game);

            std::string oldKey_in_map = "";
            CustomTrainingData* existing_pack_in_map_ptr = nullptr; // Pointer to the actual object in the map

            for (auto& [current_map_key, pack_in_map_value] : *trainingData) {
                if (pack_in_map_value.name == name_from_game) {
                    oldKey_in_map = current_map_key;
                    existing_pack_in_map_ptr = &pack_in_map_value; // Get pointer to the actual pack
                    LOG("Found pack by name '{}'. Current map key: '{}', Its .code member: '{}'",
                        name_from_game, oldKey_in_map, pack_in_map_value.code);
                    break;
                }
            }

            if (existing_pack_in_map_ptr && !code_from_game.empty() && oldKey_in_map != code_from_game) {
                // This pack (found by name) has an old key/code, and the game is giving it a new code.
                // This is a reorganization.
                LOG("Reorganizing pack. Name: '{}'. Old map key: '{}'. New game code: '{}'",
                    name_from_game, oldKey_in_map, code_from_game);

                CustomTrainingData pack_to_move = *existing_pack_in_map_ptr; // Make a copy

                // Determine the identifier for the folder on disk for the pack_to_move
                std::string current_disk_folder_identifier;
                // pack_to_move.code here is the *original* code of the pack before any in-memory changes in this function
                if (!pack_to_move.code.empty() && pack_to_move.code == oldKey_in_map) {
                    current_disk_folder_identifier = oldKey_in_map; // Folder was named after its old code
                } else if (pack_to_move.code.empty() && oldKey_in_map == pack_to_move.name) {
                    current_disk_folder_identifier = storageManager.recordingStorage.sanitizeFilename(pack_to_move.name); // Folder was named after its name
                } else {
                    // Fallback or ambiguous case: oldKey_in_map might be an old code, but pack_to_move.code might be empty or different.
                    // This implies an inconsistency or that the pack was previously keyed by name even if it had a code member.
                    // For the specific log: "old key: 16D9-73F6-A026-A3F2", this is a code.
                    // So, if oldKey_in_map looks like a code, use it.
                    if (oldKey_in_map.length() == 19 && oldKey_in_map.find_first_not_of("0123456789ABCDEF-") == std::string::npos) {
                         current_disk_folder_identifier = oldKey_in_map;
                    } else {
                         current_disk_folder_identifier = storageManager.recordingStorage.sanitizeFilename(pack_to_move.name);
                    }
                     LOG("Ambiguous old folder: using identifier '{}' based on oldKey_in_map '{}' and pack_to_move.code '{}'", current_disk_folder_identifier, oldKey_in_map, pack_to_move.code);
                }

                std::filesystem::path old_pack_disk_folder = myDataFolder / "TrainingPacks" / current_disk_folder_identifier;

                bool proceed_with_map_update = false;
                if (std::filesystem::exists(old_pack_disk_folder)) {
                    LOG("Attempting to delete old pack folder: {}", old_pack_disk_folder.string());
                    try {
                        std::filesystem::remove_all(old_pack_disk_folder);
                        LOG("Successfully deleted old pack folder.");
                        proceed_with_map_update = true;
                    } catch (const std::filesystem::filesystem_error& e) {
                        LOG("Error deleting old pack folder {}: {}. Reorganization aborted.", old_pack_disk_folder.string(), e.what());
                        return false; // Critical error, stop.
                    }
                } else {
                    LOG("Old pack folder '{}' not found. Proceeding with map update without deleting old folder.", old_pack_disk_folder.string());
                    // If the folder wasn't found, it might be an orphaned in-memory entry or first time code assignment.
                    proceed_with_map_update = true;
                }

                if (proceed_with_map_update) {
                    trainingData->erase(oldKey_in_map);          // Erase the entry under the old key
                    pack_to_move.code = code_from_game;          // NOW update the code in the copy
                    (*trainingData)[code_from_game] = pack_to_move; // Add the modified copy under the new code

                    if (currentPackKey == oldKey_in_map) {
                        currentPackKey = code_from_game;
                    }

                    // Save all data with the new structure
                    for (auto& [key_iter, value_iter] : *trainingData) { shiftToPositive(value_iter); }
                    storageManager.saveCompressedTrainingDataWithRecordings(*trainingData, myDataFolder);
                    *trainingData = storageManager.loadCompressedTrainingDataWithRecordings(myDataFolder);
                    for (auto& [key_iter, value_iter] : *trainingData) { shiftToNegative(value_iter); }
                    LOG("Reorganization successful. Pack '{}' now keyed by '{}'", name_from_game, code_from_game);
                }

            } else if (existing_pack_in_map_ptr && !code_from_game.empty() && existing_pack_in_map_ptr->code.empty()) {
                // Pack found by name (so oldKey_in_map is the name), game provides a code, and pack in map currently has no code.
                // This is assigning a code for the first time.
                LOG("Assigning code '{}' to pack '{}' (was keyed by name).", code_from_game, name_from_game);
                CustomTrainingData pack_to_update = *existing_pack_in_map_ptr; // Make a copy
                trainingData->erase(oldKey_in_map); // Erase entry keyed by name
                pack_to_update.code = code_from_game; // Assign new code to the copy
                (*trainingData)[code_from_game] = pack_to_update; // Add under new code key

                if (currentPackKey == oldKey_in_map) {
                    currentPackKey = code_from_game;
                }
                // Save all data
                for (auto& [key_iter, value_iter] : *trainingData) { shiftToPositive(value_iter); }
                storageManager.saveCompressedTrainingDataWithRecordings(*trainingData, myDataFolder);
                *trainingData = storageManager.loadCompressedTrainingDataWithRecordings(myDataFolder);
                for (auto& [key_iter, value_iter] : *trainingData) { shiftToNegative(value_iter); }
                LOG("Pack '{}' now has code and is keyed by '{}'", name_from_game, code_from_game);
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
                    
                    auto it = trainingData->find(currentTrainingData.name);
                    if (it != trainingData->end()) {
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

    //Function TAGame.GameEvent_TrainingEditor_TA.OnInit
    gameWrapper->HookEvent("Function TAGame.GameEvent_TrainingEditor_TA.OnInit",
        [this](std::string eventName) {
			justOpenedPack = true;
		});
}




void VersatileTraining::handleTrainingEditorEnter() {
    LOG("Training editor enter");
    currentTrainingData.currentEditedShot = -1;
    lockScene = false;
    justOpenedPack = true;
    

}

void VersatileTraining::handleLoadRound(ActorWrapper cw, void* params, std::string eventName) {
    if (!(isInTrainingEditor() || isInTrainingPack()))return;

    VersatileTraining::getTrainingData(cw, params, eventName);
    
    shotReplicationManager.canSpawnBot = true;
    LOG("setting can spawn bot to true");
    
    freezeForShot = currentShotState.freezeCar;
    LOG("setting freeze to shot to {} ", freezeForShot ? "true" : "false");
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
    savedReplayState= ReplayState();


    //resetting these just in case
     clampVal = 0;
     isCeiling = false;
     diagBound = 7965;
     currentXBound = 4026.0f;
     currentYBound = 5050.0f;
     frozenZVal = 0.0f;
     frozeZVal = false;
     isCarRotatable = false;
     t = 0.0f;
    

}

void VersatileTraining::handleTrainingSave() {
    if (currentTrainingData.currentEditedShot != -1) {
        

        LOG("amount of shots: {}", currentTrainingData.shots.size());

        if (currentTrainingData.shots.empty()) {
            
            LOG("Training data has no shots, not saving anything");
            return;
        }

        // Ensure currentShotState is consistent with currentTrainingData
        if (currentTrainingData.currentEditedShot < currentTrainingData.shots.size()) {
            currentTrainingData.shots[currentTrainingData.currentEditedShot] = currentShotState;
        }

        handleStopEditing();

        // Use code-based lookup first if available
        std::string searchKey = "";
        if (!currentTrainingData.code.empty()) {
            searchKey = currentTrainingData.code;
        }
        else {
            searchKey = currentTrainingData.name;
        }


        LOG("Adding new pack with key: {}", searchKey);
        (*trainingData)[searchKey] = currentTrainingData;
        

        LOG("saving num shots: {}", currentTrainingData.shots.size());

        // Rest of save process
        for (auto& [key, value] : *trainingData) {
            shiftToPositive(value);
        }
        storageManager.saveCompressedTrainingDataWithRecordings(*trainingData, myDataFolder);

        
        *trainingData = storageManager.loadCompressedTrainingDataWithRecordings(myDataFolder);
        for (auto& [key, value] : *trainingData) {
            shiftToNegative(value);
        }

        
    }
}

void VersatileTraining::handleStartEditing(ActorWrapper cw) {
    LOG("new training pack opened and editing started--------------");
    isCarRotatable = true;
    goalBlockerEligbleToBeEdited = true;

    handleLoadRound(cw, nullptr, "");

    unlockStartingVelocity = false;
    
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



    PackOverrideSettings currentOverrides; 
    bool foundSpecificOverrides = false;

    if (!code.empty() && storageManager.packOverrideSettings.count(code)) {
        currentOverrides = storageManager.packOverrideSettings.at(code);
        foundSpecificOverrides = true;
        LOG("Applying custom CVar overrides for pack code: {}", code);
    } else {
        LOG("No specific pack overrides for code '{}', or code is empty. Applying default CVar values.", code);
        
    }
    currentOverrides.ApplyCVars(cvarManager);
    if (foundSpecificOverrides) return;
    
    
    bool found = false;
    for (auto& [key, value] : *trainingData) {
        if (value.name == name) {
            
            if (justOpenedPack) {
                currentTrainingData = value;
                if (totalRounds == currentTrainingData.shots.size() && currentTrainingData.code.empty() && !code.empty()) {
                    LOG("found a potential code for this training pack");
                    pendingCode = code;
                    pendingKey = key;
                    determiningCodeSync = true;
                    if (!isWindowOpen_) {
                        cvarManager->executeCommand("togglemenu " + GetMenuName());
                    }
                }
                justOpenedPack = false;
            }
            LOG("num shots in found training pack: {}", currentTrainingData.shots.size());
            if (totalRounds < currentTrainingData.numShots) {
                LOG("resizing because num shots in training pack is less than the saved version");
                currentTrainingData.shots.resize(totalRounds);
            }
            
            currentTrainingData.customPack = true;
            currentTrainingData.currentEditedShot = currentShot;
            if (currentTrainingData.shots.size() <= currentShot) {
				LOG("current shot is out of bounds, resizing");
				currentTrainingData.shots.resize(currentShot + 1);
			}
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
        
        if (justOpenedPack) {
            LOG("just opened pack, initializing new training data");
            currentTrainingData.initCustomTrainingData(totalRounds, name,code);
            cvarManager->executeCommand("sv_training_limitboost -1");
            justOpenedPack = false;

            if (td.GetCode().ToString().empty()) {
                currentTrainingData.customPack = true;
            }
            else if (isInTrainingEditor()) {
                currentTrainingData.customPack = true;
            }
            else {
                currentTrainingData.customPack = false;
            }
        }
        currentTrainingData.currentEditedShot = currentShot;
        currentShotState = currentTrainingData.shots[currentShot];
        LOG("setting shot state");
        LOG("current training data name: {}", currentTrainingData.name);
        LOG("current training data code: {}", currentTrainingData.code);
        LOG("boost amount: {}", currentShotState.boostAmount);
        LOG("starting velocity: {}", currentShotState.startingVelocity);
        LOG("freeze car: {}", currentShotState.freezeCar ? "true" : "false");
        LOG("has jump: {}", currentShotState.hasJump ? "true" : "false");
        if (currentShotState.boostAmount == 101) {
            cvarManager->executeCommand("sv_training_limitboost -1");
        }
        else {
            cvarManager->executeCommand("sv_training_limitboost " + std::to_string(currentShotState.boostAmount));
        }

        

        
    }


}