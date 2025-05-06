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
                LOG("params is null");
                return;
            }

            ControllerInput* input = static_cast<ControllerInput*>(params);
            auto& inputs = shotReplicationManager.currentShotRecording->inputs;
            auto currentFrame = gameWrapper->GetEngine().GetPhysicsFrame();
            if (shotReplicationManager.botSpawnedTest) {
                if (!shotReplicationManager.roundStarted) {

                    if (shotReplicationManager.startPlayback) {


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

                        shotReplicationManager.frame = 0;
                        shotReplicationManager.roundStarted = true;

                        *input = NO_INPUT;
                        if (inputs[shotReplicationManager.frame].Jump) {
                            car.SetbJumped(true);
                            return;
                        }
                        else if (inputs[shotReplicationManager.frame].ActivateBoost) {//|| inputs[frame].HoldingBoost
                            //    LOG("first frame is boost, presetting");
                                //car.ForceBoost(true);
                                //input->ActivateBoost = true;
                                //*input = inputs[0];
                               //return;
                            //    //return;
                        }
                        else if (currentShotState.freezeCar) {
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
                if (shotReplicationManager.roundStarted) {
                    if (!shotReplicationManager.currentShotRecording) return; //shared_ptr to recording

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
                        shotReplicationManager.recording = true;

                    }
                }
                if (shotReplicationManager.startRecording && car.GetOwnerName() != "testplayers") {

                    inputs.push_back(*input);
                    ++shotReplicationManager.frame;
                }
                else if (shotReplicationManager.recording) {
                    LOG("ending recording on frame {}", currentFrame);
                    shotReplicationManager.recording = false;
                }
            }
        }
    );
}


void VersatileTraining::handleTrainingEditorActorModified() {
    if (editingVariances) {
        if (!lockRotation) {


            /*controllerManager.checkForButtonPress(4);
            controllerManager.checkForButtonPress(5);*/

            /*if (GetAsyncKeyState('R') & 0x8000) {
                ApplyLocalPitch(500.0f);
                LOG("applying localPitch");
            }
            if (GetAsyncKeyState('C') & 0x8000) {
                ApplyLocalPitch(-500.0f);
                LOG("applying localPitch");
            }*/
            if (GetAsyncKeyState('Q') & 0x8000) {
                rotationToApply.Roll -= 75;
            }
            if (GetAsyncKeyState('E') & 0x8000) {
                rotationToApply.Roll += 75;
            }
        }

        if (GetAsyncKeyState('2') & 0x8000) {
            currentShotState.boostAmount++;
            if (currentShotState.boostAmount > currentTrainingData.boostMax) {
                currentShotState.boostAmount = currentTrainingData.boostMax;
            }
            LOG("boost increased to {}", currentShotState.boostAmount);
        }
        if (GetAsyncKeyState('1') & 0x8000) {
            currentShotState.boostAmount--;
            if (currentShotState.boostAmount < currentTrainingData.boostMin) {
                currentShotState.boostAmount = currentTrainingData.boostMin;
            }
            LOG("boost decreased to {}", currentShotState.boostAmount);
        }
        if (GetAsyncKeyState('4') & 0x8000) {
            currentShotState.startingVelocity++;
            if (currentShotState.startingVelocity > currentTrainingData.maxVelocity) {
                currentShotState.startingVelocity = currentTrainingData.maxVelocity;
            }
            LOG("starting velocity increased to {}", currentShotState.startingVelocity);
        }
        if (GetAsyncKeyState('3') & 0x8000) {
            currentShotState.startingVelocity--;
            if (currentShotState.startingVelocity < currentTrainingData.minVelocity) {
                currentShotState.startingVelocity = currentTrainingData.minVelocity;
            }
            LOG("starting velocity decreased to {}", currentShotState.startingVelocity);
        }

        if (unlockStartingVelocity) {
            currentShotState.extendedStartingVelocity = convertRotationAndMagnitudeToVector(currentShotState.carRotation, currentShotState.startingVelocity);
        }
    }
}