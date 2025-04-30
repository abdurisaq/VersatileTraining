#include "pch.h"
#include "versatileTraining.h"

void VersatileTraining::loadHooks() {
    setupTrainingEditorHooks();
    setupTrainingShotHooks();
    setupEditorMovementHooks();
    setupGoalBlockerHooks();
    setupInputHandlingHooks();
}

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
            testCalledInStartRound = false;
            startRecording = false;
            roundStarted = false;
            playForNormalCar = false;
            auto server = gameWrapper->GetCurrentGameState();
            //delete bot if it was spawned
            if (!server) return;
            canSpawnBot = false;
            startPlayback = false;
            jumpedFirstFrame = false;
            LOG("setting can spawn bot to false");
            botSpawnedTest = false;
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

    /*bool anyInput =
                        input->Throttle != 0.0f || input->Steer != 0.0f ||
                        input->Pitch != 0.0f || input->Yaw != 0.0f || input->Roll != 0.0f ||
                        input->Jump || input->ActivateBoost || input->Handbrake;*/

    //gameWrapper->HookEventWithCaller<CarWrapper>(//TAGame.Car_TA.SetVehicleInput  Vehicle_TA
    //    "Function TAGame.Vehicle_TA.SetVehicleInput", 
    //    [this](CarWrapper car, void* params, std::string eventName) {
    //        static const ControllerInput NO_INPUT = ControllerInput{
    //            .Throttle = 0,
    //            .Steer = 0,
    //            .Pitch = 0,
    //            .Yaw = 0,
    //            .Roll = 0,
    //            .DodgeForward = 0,
    //            .DodgeStrafe = 0,
    //            .Handbrake = 0,
    //            .Jump = 0,
    //            .ActivateBoost = 0,
    //            .HoldingBoost = 0,
    //            .Jumped = 0,
    //        };
    //        auto hasAnyInput = [](const ControllerInput* input) {
    //            constexpr float epsilon = 0.00000001f;
    //            return std::abs(input->Throttle) > epsilon ||
    //                std::abs(input->Steer) > epsilon ||
    //                std::abs(input->Pitch) > epsilon ||
    //                std::abs(input->Yaw) > epsilon ||
    //                std::abs(input->Roll) > epsilon ||
    //                std::abs(input->DodgeForward) > epsilon ||
    //                std::abs(input->DodgeStrafe) > epsilon ||
    //                input->Jump ||
    //                input->ActivateBoost ||
    //                input->Handbrake ||
    //                input->HoldingBoost ||
    //                input->Jumped;
    //            };
    //        ControllerInput* input = static_cast<ControllerInput*>(params);
    //        auto& inputs = currentShotRecording->inputs;
    //        if (botSpawnedTest || playForNormalCar) {
    //            if (!roundStarted) {
    //                if (startPlayback) {//hasAnyInput(input)

    //                    
    //                    HWND hwnd = FindWindowA("LaunchUnrealUWindowsClient", "Rocket League (64-bit, DX11, Cooked)");
    //                    //PostMessage(hwnd, WM_KEYDOWN, 'D', 0);
    //                    PostMessage(hwnd, WM_KEYUP, 'D', 0);//'W'
    //                    frame = 0;
    //                    startPlayback = false;
    //                    roundStarted = true;
    //                    //*input = NO_INPUT;
    //                }
    //            }
    //        }
    //    });
    //TAGame.FXActor_Car_TA.SetBraking
    //gameWrapper->HookEventWithCaller<CarWrapper>(//Car_TA  Vehicle_TA
    //    "Function TAGame.PlayerController_TA.PlayerTick",
    //    [this](CarWrapper car, void* params, std::string eventName) {
    //        if (startPlayback ) {

    //            Vector location= car.GetLocation();
    //            if (location != currentShotRecording->initialState->location) {

    //                HWND hwnd = FindWindowA("LaunchUnrealUWindowsClient", "Rocket League (64-bit, DX11, Cooked)");
    //                //PostMessage(hwnd, WM_KEYDOWN, 'D', 0);
    //                PostMessage(hwnd, WM_KEYUP, 'D', 0);//'W'
    //                //frame = 0;
    //                startPlayback = false;
    //                roundStarted = true;
    //            }
    //        }
    //    });
    gameWrapper->HookEventWithCaller<CarWrapper>(
        "Function TAGame.Vehicle_TA.SetVehicleInput",
        [this](CarWrapper car, void* params, std::string eventName) {
            static const ControllerInput NO_INPUT = ControllerInput{
                .Throttle = 0,
                .Steer = 0,
                .Pitch = 0,
                .Yaw = 0,
                .Roll = 0,
                .DodgeForward = 0,
                .DodgeStrafe = 0,
                .Handbrake = 0,
                .Jump = 0,
                .ActivateBoost = 0,
                .HoldingBoost = 0,
                .Jumped = 0,
            };
                    auto hasAnyInput = [](const ControllerInput* input) {
                constexpr float epsilon = 0.00000001f;
                return std::abs(input->Throttle) > epsilon ||
                    std::abs(input->Steer) > epsilon ||
                    std::abs(input->Pitch) > epsilon ||
                    std::abs(input->Yaw) > epsilon ||
                    std::abs(input->Roll) > epsilon ||
                    std::abs(input->DodgeForward) > epsilon ||
                    std::abs(input->DodgeStrafe) > epsilon ||
                    input->Jump ||
                    input->ActivateBoost ||
                    input->Handbrake ||
                    input->HoldingBoost ||
                    input->Jumped;
                };


            if (!params) {
                LOG("params is null");
                return;
            }

            ControllerInput* input = static_cast<ControllerInput*>(params);
            auto& inputs = currentShotRecording->inputs;
            auto currentFrame = gameWrapper->GetEngine().GetPhysicsFrame();
            if (botSpawnedTest || playForNormalCar) {
                if (!roundStarted) {

                    if (startPlayback) {

                        //HWND hwnd = FindWindowA("LaunchUnrealUWindowsClient", "Rocket League (64-bit, DX11, Cooked)");
                        //PostMessage(hwnd, WM_KEYDOWN, 'D', 0);
                        //PostMessage(hwnd, WM_KEYUP, 'D', 0);//'W'
                        LOG(" Applying input - Throttle: {:.7f}, Steer : {:.7}, Pitch : {:.7f}, Yaw : {:.7f}, Roll : {:.7f}, DodgeForward : {:.7f}, DodgeStrafe : {:.7f}, Handbrake{}, Jump{}, ActivateBoost{}, HoldingBoost{}, Jumped{}",
                            inputs[0].Throttle, inputs[0].Steer, inputs[0].Pitch, inputs[0].Yaw, inputs[0].Roll, inputs[0].DodgeForward, inputs[0].DodgeStrafe, inputs[0].Handbrake ? "true" : "false", inputs[0].Jump ? "true" : "false", inputs[0].ActivateBoost ? "true" : "false", inputs[0].HoldingBoost ? "true" : "false", inputs[0].Jumped ? "true" : "false");


                        INPUT inputsToSend[2] = {};

                        if (inputs[0].Throttle < -0.0000001f) {
                            LOG("first keypress goes backwards");
                            // Set up a key down event
                            inputsToSend[0].type = INPUT_KEYBOARD;
                            inputsToSend[0].ki.wVk = 'S';  // Virtual-key code for 'W'
                            inputsToSend[0].ki.dwFlags = 0; // 0 for key press

                            // Set up a key up event
                            inputsToSend[1].type = INPUT_KEYBOARD;
                            inputsToSend[1].ki.wVk = 'S';  // Same virtual key code
                            inputsToSend[1].ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for release
                        }
                        else if (inputs[0].Steer < -0.00001f) {
                            LOG("first keypress goes left");
                            // Set up a key down event
                            inputsToSend[0].type = INPUT_KEYBOARD;
                            inputsToSend[0].ki.wVk = 'A';  // Virtual-key code for 'W'
                            inputsToSend[0].ki.dwFlags = 0; // 0 for key press

                            // Set up a key up event
                            inputsToSend[1].type = INPUT_KEYBOARD;
                            inputsToSend[1].ki.wVk = 'A';  // Same virtual key code
                            inputsToSend[1].ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for release
                        }
                        else if (inputs[0].Steer > 0.00001f)
                        {
                            LOG("first keypress goes right");
                            // Set up a key down event
                            inputsToSend[0].type = INPUT_KEYBOARD;
                            inputsToSend[0].ki.wVk = 'D';  // Virtual-key code for 'W'
                            inputsToSend[0].ki.dwFlags = 0; // 0 for key press

                            //// Set up a key up event
                            inputsToSend[1].type = INPUT_KEYBOARD;
                            inputsToSend[1].ki.wVk = 'D';  // Same virtual key code
                            inputsToSend[1].ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for release
                        }
                        else
                        {
                            LOG("first keypress goes forward");
                            // Set up a key down event
                            inputsToSend[0].type = INPUT_KEYBOARD;
                            inputsToSend[0].ki.wVk = 'W';  // Virtual-key code for 'W'
                            inputsToSend[0].ki.dwFlags = 0; // 0 for key pressinputsToSend[1].type = INPUT_KEYBOARD;

                            inputsToSend[1].type = INPUT_KEYBOARD;
                            inputsToSend[1].ki.wVk = 'W';  // Same virtual key code
                            inputsToSend[1].ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for release


                        }

                        SendInput(2, inputsToSend, sizeof(INPUT));

                        frame = 0;
                        roundStarted = true;

                        *input = NO_INPUT;
                        if (inputs[frame].Jump) {
                            car.SetbJumped(true);
                            return;
                        }
                        else if (inputs[frame].ActivateBoost ) {//|| inputs[frame].HoldingBoost
                        //    LOG("first frame is boost, presetting");
                            /*car.ForceBoost(true);*/
                            //input->ActivateBoost = true;
                            //*input = inputs[0];
                        //    return;
                        //    //return;
                        }
                        else if (freezeCar) {
                            LOG("car is frozen, setting jump to false");
                            car.SetbDoubleJumped(false);

                            if (std::abs(inputs[0].Throttle) > 0.00001f || std::abs(inputs[0].Steer) > 0.00001f)
                            {
                                return;
                            }
                            //return;
                        }
                        /*if (std::abs(inputs[0].Throttle) > 0.00001f || std::abs(inputs[0].Steer) > 0.00001f)
                        {
                            return;
                        }*/
                        return;
                        LOG("any wheel touching ground {}", car.AnyWheelTouchingGround() ? "true" : "false");

                    }
                }
                if (roundStarted) {
                    if (!currentShotRecording) return; //shared_ptr to recording

                    if (frame <= inputs.size()) {

                        *input = inputs[frame]; //-1 

                        frame++;
                    }
                    else {

                        LOG("Frame {} out of bounds, size: {}", frame, inputs.size());

                        roundStarted = false;
                    }
                }
            }
            else {
                if (primedToStartRecording) {
                    if ( testCalledInStartRound) {//hasAnyInput(input) &&
                        LOG("Starting recording from first detected input frame is  {}", currentFrame);
                        startRecording = true;
                        primedToStartRecording = false;
                        recording = true;

                    }
                }
                if (startRecording && car.GetOwnerName() != "testplayers") {

                    inputs.push_back(*input);
                    ++frame;
                }
                else if (recording) {
                    LOG("ending recording on frame {}", currentFrame);
                    recording = false;
                }
            }
        }
    );

    

    gameWrapper->HookEventWithCallerPost<ActorWrapper>(
        "Function TAGame.TrainingEditorMetrics_TA.TrainingEditorExit",
        [this](ActorWrapper cw, void* params, std::string eventName) {
            handleTrainingEditorExit();
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

void VersatileTraining::setupEditorMovementHooks() {
    gameWrapper->HookEventWithCallerPost<ActorWrapper>(
        "Function TAGame.GFxHUD_TA.UpdateCarData",
        [this](ActorWrapper cw, void* params, std::string eventName) {
            handleUpdateCarData(cw);
        });

    gameWrapper->HookEvent(
        "Function GameEvent_Soccar_TA.Active.StartRound",
        [this](std::string eventName) {
            handleStartRound();
        });

    gameWrapper->HookEventWithCaller<ActorWrapper>(
        "Function TAGame.GameEditor_Actor_TA.EditorMoveToLocation",
        [this](ActorWrapper cw, void* params, std::string eventName) {
            handleEditorMoveToLocation(cw, params);
        });

    gameWrapper->HookEventWithCallerPost<ActorWrapper>(
        "Function TAGame.GameEditor_Actor_TA.EditorSetRotation",
        [this](ActorWrapper cw, void* params, std::string eventName) {
            handleEditorSetRotation(cw);
        });

    gameWrapper->HookEventWithCallerPost<ActorWrapper>(
        "Function TAGame.PlayerController_TA.GetRotateActorCameraOffset",
        [this](ActorWrapper cw, void* params, std::string eventName) {
            handleGetRotateActorCameraOffset(cw);
        });
}

void VersatileTraining::setupGoalBlockerHooks() {
    gameWrapper->HookEventWithCaller<ActorWrapper>(
        "Function TAGame.Ball_GameEditor_TA.EditingBegin",
        [this](ActorWrapper cw, void* params, std::string eventName) {
            handleBallEditingBegin();
        });

    gameWrapper->HookEventWithCaller<ActorWrapper>(
        "Function PlayerController_TA.Editing.StopEditing",
        [this](ActorWrapper cw, void* params, std::string eventName) {
            handleStopEditingGoalBlocker();
        });

    gameWrapper->HookEventWithCaller<ActorWrapper>(
        "Function TAGame.CameraState_GameEditor_TA.UpdateFlyPOV",
        [this](ActorWrapper cw, void* params, std::string eventName) {
            handleUpdateFlyPOV();
        });
}

void VersatileTraining::setupInputHandlingHooks() {
    gameWrapper->HookEvent(
        "Function TAGame.PlayerController_TA.EventTrainingEditorActorModified",
        [this](std::string eventName) {
            handleTrainingEditorActorModified();
        });
}

// Handler Implementations

void VersatileTraining::handleTrainingEditorEnter() {
    LOG("Training editor enter");
    currentTrainingData.currentEditedShot = -1;
}

void VersatileTraining::handleLoadRound(ActorWrapper cw, void* params, std::string eventName) {
    
    VersatileTraining::getTrainingData(cw, params, eventName);
    canSpawnBot = true;
    LOG("setting can spawn bot to true");
    freezeForShot = freezeCar;
    frozeZVal = !freezeCar;
    lockRotation = true;
    appliedStartingVelocity = false;
    editingVariances = false;
    appliedWallClamping = false;
    editingGoalBlocker = false;
}

void VersatileTraining::handleTrainingEditorExit() {
    currentTrainingData.currentEditedShot = -1;
    editingGoalBlocker = false;
    goalBlockerEligbleToBeEdited = false;
    goalAnchors = { false, false };
}

void VersatileTraining::handleTrainingSave() {
    if (currentTrainingData.currentEditedShot != -1) {
        currentTrainingData.boostAmounts[currentTrainingData.currentEditedShot] = tempBoostAmount;
        currentTrainingData.startingVelocity[currentTrainingData.currentEditedShot] = tempStartingVelocity;
        currentTrainingData.freezeCar[currentTrainingData.currentEditedShot] = freezeCar;
        trainingData[currentPackKey] = currentTrainingData;

        LOG("saving num shots: {}", currentTrainingData.numShots);

        for (auto& [key, value] : trainingData) {
            shiftVelocitiesToPositive(value.startingVelocity);
            shiftGoalBlockerToPositive(value.goalBlockers);
        }
        SaveCompressedTrainingData(trainingData, saveFilePath);
        trainingData = LoadCompressedTrainingData(saveFilePath);
        for (auto& [key, value] : trainingData) {
            shiftVelocitiesToNegative(value.startingVelocity);
            shiftGoalBlockerToNegative(value.goalBlockers);
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
        handleExistingTrainingData(currentShot);
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

    handleNewTrainingData(currentShot);
}

void VersatileTraining::handleExistingTrainingData(int currentShot) {

    

    LOG("already loaded, skipping searching training data");
    LOG("currentShot: {}", currentTrainingData.currentEditedShot);
    LOG("amount of shots in found training data: {}", currentTrainingData.numShots);

    while (currentShot >= currentTrainingData.numShots) {
        LOG("adding shot");
        currentTrainingData.addShot();
        LOG("now this many shots in training data: {}", currentTrainingData.numShots);
    }

    tempBoostAmount = currentTrainingData.boostAmounts[currentShot];
    tempStartingVelocity = currentTrainingData.startingVelocity[currentShot];
    currentTrainingData.currentEditedShot = currentShot;
    freezeCar = currentTrainingData.freezeCar[currentShot];

    std::pair<Vector, Vector> goalBlockerPosToAssign = currentTrainingData.goalBlockers[currentShot];
    /*goalAnchors.first = !(goalBlockerPosToAssign.first.X == 0.f || goalBlockerPosToAssign.first.Z == 0.f);
    goalAnchors.second = !(goalBlockerPosToAssign.second.X == 0.f || goalBlockerPosToAssign.second.Z == 0.f);*/
    goalAnchors = currentTrainingData.goalAnchors[currentShot];
    goalBlockerPos = currentTrainingData.goalBlockers[currentShot];
    LOG("pulled goalblocker, x1:{}, z1:{} x2:{} z2{}. setting anchor first to : {}, and send to : {}", goalBlockerPos.first.X, goalBlockerPos.first.Z, goalBlockerPos.second.X, goalBlockerPos.second.Z, goalAnchors.first ? "true" : "false", goalAnchors.second ? "true" : "false");
}

void VersatileTraining::handleNewTrainingData(int currentShot) {
    LOG("currentShot: {}", currentShot);
    LOG("amount of shots in found training data: {}", currentTrainingData.numShots);

    while (currentShot >= currentTrainingData.numShots) {
        LOG("adding shot");
        currentTrainingData.addShot();
        LOG("now this many shots in training data: {}", currentTrainingData.numShots);
    }

    tempBoostAmount = currentTrainingData.boostAmounts[currentShot];
    tempStartingVelocity = currentTrainingData.startingVelocity[currentShot];
    currentTrainingData.currentEditedShot = currentShot;
    freezeCar = currentTrainingData.freezeCar[currentShot];

    std::pair<Vector, Vector> goalBlockerPosToAssign = currentTrainingData.goalBlockers[currentShot];
    //goalAnchors.first = !(goalBlockerPosToAssign.first.X == 0.f || goalBlockerPosToAssign.first.Z == 0.f);//&& (blockerToAssign.first != Vector{ 0,0,0 })
    //goalAnchors.second = !(goalBlockerPosToAssign.second.X == 0.f || goalBlockerPosToAssign.second.Z == 0.f);
    goalAnchors = currentTrainingData.goalAnchors[currentShot];
    goalBlockerPos = currentTrainingData.goalBlockers[currentShot];
    LOG("pulled goalblocker, x1:{}, z1:{} x2:{} z2{}. setting anchor first to : {}, and send to : {}", goalBlockerPos.first.X, goalBlockerPos.first.Z, goalBlockerPos.second.X, goalBlockerPos.second.Z, goalAnchors.first ? "true" : "false", goalAnchors.second ? "true" : "false");
}

void VersatileTraining::handleCreateRound() {
    currentTrainingData.addShot();
}

void VersatileTraining::handleDeleteRound(TrainingEditorWrapper cw) {
    int shotToRemove = cw.GetActiveRoundNumber();
    if (shotToRemove >= 0 && shotToRemove < currentTrainingData.numShots) {
        LOG("Removing shot: {}", shotToRemove);
        currentTrainingData.boostAmounts.erase(currentTrainingData.boostAmounts.begin() + shotToRemove);
        currentTrainingData.startingVelocity.erase(currentTrainingData.startingVelocity.begin() + shotToRemove);
        currentTrainingData.freezeCar.erase(currentTrainingData.freezeCar.begin() + shotToRemove);
        currentTrainingData.goalBlockers.erase(currentTrainingData.goalBlockers.begin() + shotToRemove);
        currentTrainingData.numShots--;
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
    LOG("copying a shot that is frozen ? {}", currentTrainingData.freezeCar[currentShot] ? "true" : "false");

    int boostToCopy = currentTrainingData.boostAmounts[currentShot];
    int startingVelocityToCopy = currentTrainingData.startingVelocity[currentShot];
    bool freezeToCopy = currentTrainingData.freezeCar[currentShot];
    currentTrainingData.addShot(boostToCopy, startingVelocityToCopy, freezeToCopy);
}

void VersatileTraining::handleStartPlayTest() {
    if (!currentTrainingData.customPack) return;

    LOG("setting boost amount in start play test to {}", tempBoostAmount);
    currentTrainingData.boostAmounts[currentTrainingData.currentEditedShot] = tempBoostAmount;
    LOG("setting starting velocity in start play test to {}", tempStartingVelocity);
    currentTrainingData.startingVelocity[currentTrainingData.currentEditedShot] = tempStartingVelocity;
    LOG("setting freeze car in start play test to {}", freezeCar ? "false" : "true");
    currentTrainingData.freezeCar[currentTrainingData.currentEditedShot] = freezeCar;
    trainingData[currentTrainingData.name] = currentTrainingData;
}

void VersatileTraining::handleStopEditing() {
    if (!currentTrainingData.customPack) return;

    LOG("stopped editing");
    isCarRotatable = false;
    lockRotation = true;
    editingVariances = false;

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
    LOG("temp boost amount: {}", tempBoostAmount);

    currentTrainingData.boostAmounts[currentTrainingData.currentEditedShot] = tempBoostAmount;
    currentTrainingData.startingVelocity[currentTrainingData.currentEditedShot] = tempStartingVelocity;
    currentTrainingData.freezeCar[currentTrainingData.currentEditedShot] = freezeCar;
    currentTrainingData.goalBlockers[currentTrainingData.currentEditedShot] = goalBlockerPos;
    currentTrainingData.goalAnchors[currentTrainingData.currentEditedShot] = goalAnchors;
    goalBlockerPos = { { 0, 5140, 0 }, { 0, 5140, 0 } };
    goalAnchors = { false, false };
    trainingData[currentTrainingData.name] = currentTrainingData;

    LOG("adding shot training data: {}, boost amount: {}, starting velocity: {}",
        currentTrainingData.currentEditedShot,
        currentTrainingData.boostAmounts[currentTrainingData.currentEditedShot],
        currentTrainingData.startingVelocity[currentTrainingData.currentEditedShot]);
}

void VersatileTraining::handleBallEditingEnd() {
    LOG("car is being edited");
    editingVariances = true;
}

void VersatileTraining::handleEditorModeEndState() {
    editingVariances = false;
}

void VersatileTraining::handleGameEditorActorEditingEnd() {
    LOG("ball is being edited");
    editingVariances = false;
    lockRotation = true;
}

void VersatileTraining::handleEndPlayTest() {
    isCarRotatable = true;
}

void VersatileTraining::handleUpdateCarData(ActorWrapper cw) {
    if (!currentTrainingData.customPack) return;

    cw.SetbCollideWorld(0);

    if (!gameWrapper->IsInCustomTraining()) return;

    ServerWrapper server = gameWrapper->GetCurrentGameState();
    if (!server) return;

    ActorWrapper car = server.GetGameCar();
    if (!car) return;

    Rotator rot = car.GetRotation();
    Vector loc = car.GetLocation();

    if (!appliedWallClamping) {
        LOG("applying clamping to wall");
        Vector loc2 = getClampChange(loc, rot);
        car.SetLocation(loc2);
        appliedWallClamping = true;
    }

    if (freezeForShot) {
        handleFreezeCar(car, loc, rot);
    }
    else if (!freezeForShot && !appliedStartingVelocity) {
        handleUnfrozenCar(car, loc, rot);
    }
}

void VersatileTraining::handleFreezeCar(ActorWrapper car, Vector loc, Rotator rot) {
    if (rot.Pitch != carRotationUsed.Pitch || rot.Yaw != carRotationUsed.Yaw) {
       

        if (!frozeZVal) {
            frozenZVal = loc.Z;
            frozeZVal = true;
        }

        float pitchRad = (rot.Pitch / 16201.0f) * (PI / 2);
        float yawRad = (rot.Yaw / 32768.0f) * PI;
        float z = sinf(pitchRad);
        float y = cosf(pitchRad) * sinf(yawRad);
        float x = cosf(pitchRad) * cosf(yawRad);
        Vector unitVector = { x, y, z };
        int velocity = currentTrainingData.startingVelocity[currentTrainingData.currentEditedShot];
        startingVelocityTranslation = unitVector * velocity;
    }

    Vector loc2 = getClampChange(loc, rot);
    if (loc2.X != 0 || loc2.Y != 0 || loc2.Z != 0) {
        LOG("old location - X: {}, Y: {}, Z: {}", loc.X, loc.Y, loc.Z);
        LOG("new location - X: {}, Y: {}, Z: {}", loc2.X, loc2.Y, loc2.Z);
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

void VersatileTraining::handleStartRound() {

    

    if (gameWrapper->IsInCustomTraining()) {
        testCalledInStartRound = true;
       


        if (!currentTrainingData.customPack) return;
        freezeForShot = false;
        frozeZVal = false;
        appliedStartingVelocity = false;

        ServerWrapper server = gameWrapper->GetCurrentGameState();
        if (!server) return;
        ActorWrapper car = server.GetGameCar();
        if (!car) return;

        Rotator rot = car.GetRotation();

        float pitchRad = (rot.Pitch / 16201.0f) * (PI / 2);
        float yawRad = (rot.Yaw / 32768.0f) * PI;
        float z = sinf(pitchRad);
        float y = cosf(pitchRad) * sinf(yawRad);
        float x = cosf(pitchRad) * cosf(yawRad);
        Vector unitVector = { x, y, z };

        int velocity = currentTrainingData.startingVelocity[currentTrainingData.currentEditedShot];
        if (velocity == 0) return;
        startingVelocityTranslation = unitVector * velocity;
        Vector stickingVelocity = getStickingVelocity(rot);
        car.SetVelocity(startingVelocityTranslation + stickingVelocity);

        LOG("Calculated new velocity in StartRound");
    }
}

struct pExecEditorMoveToLocaction
{
    struct Vector NewLocation;
    uint32_t ReturnValue : 1;
};

void VersatileTraining::handleEditorMoveToLocation(ActorWrapper cw, void* params) {
    auto* p = reinterpret_cast<pExecEditorMoveToLocaction*>(params);

    const float diag = 7950;
    float diagReduction = 0;
    diagBound = 7950;
    const float ybuff = 5050;
    const float xbuff = 4026;
    const float zMin = 150;
    const float upperRampSize = 450;
    const float zMax = 2044 - upperRampSize;
    const float baseRampDepth = 150;
    const float baseBoundaryShrink = 150;
    const float ceilingScale = upperRampSize / baseRampDepth;

    cw.SetbCollideWorld(0);

    p->NewLocation.Z = std::clamp(p->NewLocation.Z, 0.0f, 2005.0f);

    currentXBound = xbuff;
    currentYBound = ybuff;

    if (p->NewLocation.Z < zMin || p->NewLocation.Z > zMax) {
        isCeiling = (p->NewLocation.Z > zMax);
        float rampDepth = isCeiling ? baseRampDepth * ceilingScale : baseRampDepth;
        float boundaryShrink = isCeiling ? baseBoundaryShrink * ceilingScale : baseBoundaryShrink;

        t = isCeiling
            ? (p->NewLocation.Z - zMax) / rampDepth
            : (zMin - p->NewLocation.Z) / rampDepth;
        t = std::clamp(t, 0.0f, 1.0f);

        float maxShrinkRatio = (xbuff - boundaryShrink) / xbuff;
        float circleFactor = sqrt(1.0f - t * t * (1 - maxShrinkRatio * maxShrinkRatio));
        currentXBound = xbuff * circleFactor;
        currentYBound = ybuff * circleFactor;
        diagBound = diag * circleFactor;
    }

    auto clampToDiagonal = [](float& x, float& y, float A, float B, float C) {
        float d = (A * x + B * y + C) / (A * A + B * B);
        x = x - A * d;
        y = y - B * d;
        };

    // Topleft
    if (diagBound < p->NewLocation.X + p->NewLocation.Y) {
        clampToDiagonal(p->NewLocation.X, p->NewLocation.Y, 1, 1, -diagBound);
    }
    // downright
    else if (p->NewLocation.X + p->NewLocation.Y < -diagBound) {
        clampToDiagonal(p->NewLocation.X, p->NewLocation.Y, 1, 1, diagBound);
    }
    // Topright
    else if ((p->NewLocation.X - p->NewLocation.Y) > diagBound) {
        clampToDiagonal(p->NewLocation.X, p->NewLocation.Y, 1, -1, -diagBound);
    }
    // downleft
    else if ((p->NewLocation.Y - p->NewLocation.X) > diagBound) {
        clampToDiagonal(p->NewLocation.X, p->NewLocation.Y, 1, -1, diagBound);
    }

    // apply bounding
    p->NewLocation.X = std::clamp(p->NewLocation.X, -currentXBound, currentXBound);
    p->NewLocation.Y = std::clamp(p->NewLocation.Y, -currentYBound, currentYBound);
}

void VersatileTraining::handleEditorSetRotation(ActorWrapper cw) {
    if (editingVariances) {
        if (!cw || cw.IsNull()) {
            LOG("Server not found");
            return;
        }

        Vector loc = cw.GetLocation();
       // Rotator rot = cw.GetRotation();
        

        //if (!lockRotation) {
            Rotator rot = cw.GetRotation();
            

            if (rotationToApply.Pitch == 0) {
                rot.Pitch = currentRotation.Pitch;
            }
            else {
                
                rot.Pitch = rotationToApply.Pitch;
            }

            rot.Roll += rotationToApply.Roll;
            rotationToApply = { 0,0,0 };
            cw.SetRotation(rot);
            currentRotation = rot;
            currentLocation = loc;

            Rotator rot1 = checkForClamping(loc, rot);
            if (localRotation.Roll != 0 && localRotation.Pitch != 0 && localRotation.Yaw != 0) {
                cw.SetRotation(localRotation);
                localRotation = { 0,0,0 };
                currentRotation = localRotation;
            }

            if (rot1.Yaw != 0 && rot.Pitch != 0 && rot.Roll != 0) {
               
                cw.SetRotation({ rot1.Pitch,rot1.Yaw,rot1.Roll });
            }
       // }
        /*else if(clampVal !=0) {
            Rotator rot1 = checkForClamping(loc, rot);
            cw.SetRotation(rot1);
        }*/
    }
}

void VersatileTraining::handleGetRotateActorCameraOffset(ActorWrapper cw) {
    if (editingVariances && !lockRotation) {
        if (!cw || cw.IsNull()) {
            LOG("Server not found");
            return;
        }

        if (clampVal != 5) {
            Rotator rot = gameWrapper->GetCamera().GetRotation();
            rotationToApply.Pitch = rot.Pitch;
        }
    }
}

void VersatileTraining::handleBallEditingBegin() {
    if (goalBlockerEligbleToBeEdited) {
        goalBlockerEligbleToBeEdited = false;
        editingGoalBlocker = false;
    }
}

void VersatileTraining::handleStopEditingGoalBlocker() {
    if (goalBlockerEligbleToBeEdited) {
        goalBlockerEligbleToBeEdited = false;
        editingGoalBlocker = false;
    }
}

void VersatileTraining::handleUpdateFlyPOV() {
    if (editingGoalBlocker) {
        Rotator rot = gameWrapper->GetCamera().GetRotation();
        Vector loc = gameWrapper->GetCamera().GetLocation();
        bool rotChanged = false;

        if (!savedPastCamera) {
            previousLocBeforeGoalEdit = loc;
            previousRotBeforeGoalEdit = rot;
            savedPastCamera = true;
        }

        /*LOG("Camera - Pitch: {}, Yaw: {}, Roll: {}", rot.Pitch, rot.Yaw, rot.Roll);
        LOG("Camera - X: {}, Y: {}, Z: {}", loc.X, loc.Y, loc.Z);*/

        loc.Y = 4000.f;
        loc.X = 0.f;
        loc.Z = 321.f;

        float centerYaw = 16384.f;
        float maxYawRange = 7000.f;
        float normalizedYawDist = std::abs(rot.Yaw - centerYaw) / maxYawRange;
        normalizedYawDist = std::clamp(normalizedYawDist, 0.f, 1.f);

        float extraPitch = (1.f - normalizedYawDist) * 600.f;
        float allowedPitch = 2400.f + extraPitch;

        if (rot.Pitch > allowedPitch) {
            rot.Pitch = allowedPitch;
            rotChanged = true;
        }
        else if (rot.Pitch < -allowedPitch) {
            rot.Pitch = -allowedPitch;
            rotChanged = true;
        }

        if (rot.Yaw < 9384.f) {
            rot.Yaw = 9384.f;
            rotChanged = true;
        }
        else if (rot.Yaw > 23384.f) {
            rot.Yaw = 23384.f;
            rotChanged = true;
        }

        gameWrapper->GetCamera().SetLocation(loc);
        if (rotChanged) {
            gameWrapper->GetCamera().SetRotation(rot);
        }
    }
    else if (!editingGoalBlocker && savedPastCamera) {
        LOG("applying previous camera location and rotation, and setting saved to false");
        gameWrapper->GetCamera().SetLocation(previousLocBeforeGoalEdit);
        gameWrapper->GetCamera().SetRotation(previousRotBeforeGoalEdit);
        previousLocBeforeGoalEdit = { 0, 0, 0 };
        previousRotBeforeGoalEdit = { 0, 0, 0 };
        savedPastCamera = false;
    }
}

void VersatileTraining::handleTrainingEditorActorModified() {
    if (editingVariances) {
        if (!lockRotation) {
            if (GetAsyncKeyState('R') & 0x8000) {
                ApplyLocalPitch(500.0f);
                LOG("applying localPitch");
            }
            if (GetAsyncKeyState('C') & 0x8000) {
                ApplyLocalPitch(-500.0f);
                LOG("applying localPitch");
            }
            if (GetAsyncKeyState('Q') & 0x8000) {
                rotationToApply.Roll -= 75;
            }
            if (GetAsyncKeyState('E') & 0x8000) {
                rotationToApply.Roll += 75;
            }
        }

        if (GetAsyncKeyState('2') & 0x8000) {
            tempBoostAmount++;
            if (tempBoostAmount > boostMax) {
                tempBoostAmount = boostMax;
            }
            LOG("boost increased to {}", tempBoostAmount);
        }
        if (GetAsyncKeyState('1') & 0x8000) {
            tempBoostAmount--;
            if (tempBoostAmount < boostMin) {
                tempBoostAmount = boostMin;
            }
            LOG("boost decreased to {}", tempBoostAmount);
        }
        if (GetAsyncKeyState('4') & 0x8000) {
            tempStartingVelocity++;
            if (tempStartingVelocity > maxVelocity) {
                tempStartingVelocity = maxVelocity;
            }
            LOG("starting velocity increased to {}", tempStartingVelocity);
        }
        if (GetAsyncKeyState('3') & 0x8000) {
            tempStartingVelocity--;
            if (tempStartingVelocity < minVelocity) {
                tempStartingVelocity = minVelocity;
            }
            LOG("starting velocity decreased to {}", tempStartingVelocity);
        }
    }
}