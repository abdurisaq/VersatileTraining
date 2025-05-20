#include "pch.h"
#include "src/core/versatileTraining.h"


void VersatileTraining::setupInputHandlingHooks() {
    gameWrapper->HookEvent(
        "Function TAGame.PlayerController_TA.EventTrainingEditorActorModified",
        [this](std::string eventName) {
            handleTrainingEditorActorModified();
        });

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

                return;
            }

            ControllerInput* input = static_cast<ControllerInput*>(params); // currentShotState.recording.inputs
            auto& inputs = shotReplicationManager.currentShotRecording.inputs;//shotReplicationManager.currentShotRecording.inputs
            auto currentFrame = gameWrapper->GetEngine().GetPhysicsFrame();
            if (shotReplicationManager.botSpawnedTest) {
                if (!shotReplicationManager.roundStarted) {

                    if (shotReplicationManager.startPlayback) {


                        LOG(" Applying input - Throttle: {:.7f}, Steer : {:.7}, Pitch : {:.7f}, Yaw : {:.7f}, Roll : {:.7f}, DodgeForward : {:.7f}, DodgeStrafe : {:.7f}, Handbrake{}, Jump{}, ActivateBoost{}, HoldingBoost{}, Jumped{}",
                            inputs[0].Throttle, inputs[0].Steer, inputs[0].Pitch, inputs[0].Yaw, inputs[0].Roll, inputs[0].DodgeForward, inputs[0].DodgeStrafe, inputs[0].Handbrake ? "true" : "false", inputs[0].Jump ? "true" : "false", inputs[0].ActivateBoost ? "true" : "false", inputs[0].HoldingBoost ? "true" : "false", inputs[0].Jumped ? "true" : "false");


                        

                        shotReplicationManager.frame = 0;
                        shotReplicationManager.roundStarted = true;
                        
                    }
                    /*else {
                        *input = inputs[0];
                    }*/
                }
                if (shotReplicationManager.roundStarted) {
                    if (currentShotState.recording.carBody == 0) {
                        LOG("no inputs to play back, returning");
                        return;
                    }

                    if (shotReplicationManager.frame <= inputs.size()) {

                        *input = inputs[shotReplicationManager.frame]; //-1 

                        shotReplicationManager.frame++;
                    }
                    else {

                        LOG("Frame {} out of bounds, size: {}", shotReplicationManager.frame, inputs.size());

                        shotReplicationManager.roundStarted = false;
                    }
                }
            }
            else {
                if (shotReplicationManager.primedToStartRecording) {
                    if (shotReplicationManager.testCalledInStartRound) {//hasAnyInput(input) &&
                        LOG("Starting recording from first detected input frame is  {}", currentFrame);
                        shotReplicationManager.startRecording = true;
                        shotReplicationManager.primedToStartRecording = false;
                        shotReplicationManager.currentShotRecording.initialState.location = car.GetLocation();
                        shotReplicationManager.currentShotRecording.initialState.rotation = car.GetRotation();
                        shotReplicationManager.currentShotRecording.initialState.velocity = car.GetVelocity();
                        shotReplicationManager.recording = true;

                    }
                }
                if (shotReplicationManager.startRecording && car.GetOwnerName() != "testplayers") {

                    inputs.push_back(*input);
                   
                }
                else if (shotReplicationManager.recording) {
                    LOG("ending recording on frame {}", currentFrame);
                    shotReplicationManager.recording = false;
                }
            }
        }
    );



    gameWrapper->HookEventWithCaller<PlayerControllerWrapper>(
		"Function TAGame.PlayerController_TA.OnOpenPauseMenu",
        [this](PlayerControllerWrapper cw, void* params, std::string eventName) {
            settingsOpen = true;
		});

    gameWrapper->HookEventWithCaller<PlayerControllerWrapper>(
		"Function Engine.ControllerLayoutStack.Pop",
        [this](PlayerControllerWrapper cw, void* params, std::string eventName) {
            settingsOpen = false;
		});
    gameWrapper->HookEventWithCaller<PlayerControllerWrapper>(
        "Function ProjectX.GameInfo_X.AddPauser",
        [this](PlayerControllerWrapper cw, void* params, std::string eventName) {
            settingsOpen = true;
        });

    gameWrapper->HookEventWithCaller<PlayerControllerWrapper>(
        "Function ProjectX.GameInfo_X.RemovePauser",
        [this](PlayerControllerWrapper cw, void* params, std::string eventName) {
            settingsOpen = false;
        });


}


void VersatileTraining::handleTrainingEditorActorModified() {
    if (editingVariances) {
        if (!lockRotation) {

            if (GetAsyncKeyState(specialKeybinds.rollLeft) & 0x8000) {
                rotationToApply.Roll -= 75;
            }
            if (GetAsyncKeyState(specialKeybinds.rollRight) & 0x8000) {
                rotationToApply.Roll += 75;
            }
        }

        if (GetAsyncKeyState(specialKeybinds.increaseBoost) & 0x8000) {
            increaseBoostKeyPressCounter++;
            if (increaseBoostKeyPressCounter >= BOOST_ADJUST_DELAY_FRAMES) {
                currentShotState.boostAmount++;
                if (currentShotState.boostAmount > currentTrainingData.boostMax) {
                    currentShotState.boostAmount = currentTrainingData.boostMax;
                }
                increaseBoostKeyPressCounter = 0; 
            }
        }
        else {
            increaseBoostKeyPressCounter = 0; 
        }

        if (GetAsyncKeyState(specialKeybinds.decreaseBoost) & 0x8000) {
            decreaseBoostKeyPressCounter++;
            if (decreaseBoostKeyPressCounter >= BOOST_ADJUST_DELAY_FRAMES) {
                currentShotState.boostAmount--;
                if (currentShotState.boostAmount < currentTrainingData.boostMin) {
                    currentShotState.boostAmount = currentTrainingData.boostMin;
                }
                decreaseBoostKeyPressCounter = 0; // Reset counter after action
            }
        }
        else {
            decreaseBoostKeyPressCounter = 0; 
        }
        

        if (unlockStartingVelocity) {
            if (GetAsyncKeyState(specialKeybinds.decreaseVelocity) & 0x8000) {
                currentShotState.startingVelocity--;
                if (currentShotState.startingVelocity < currentTrainingData.minVelocity) {
                    currentShotState.startingVelocity = currentTrainingData.minVelocity;
                }
                
            }
            if (GetAsyncKeyState(specialKeybinds.increaseVelocity) & 0x8000) {
                currentShotState.startingVelocity++;
                if (currentShotState.startingVelocity > currentTrainingData.maxVelocity) {
                    currentShotState.startingVelocity = currentTrainingData.maxVelocity;
                }
                
            }
            currentShotState.extendedStartingVelocity = convertRotationAndMagnitudeToVector(currentShotState.carRotation, currentShotState.startingVelocity);
        }
    }
}