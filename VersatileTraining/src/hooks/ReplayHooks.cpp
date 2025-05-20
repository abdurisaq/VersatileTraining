#include "pch.h"
#include "src/core/versatileTraining.h"

void VersatileTraining::replayHooks() {

    struct GetFocusCarParams {
        uintptr_t ReturnValue;
    };

    gameWrapper->HookEvent("Function TAGame.MenuSequence_TA.EnterSequence", [this](std::string eventName) {
        canSpawnWelcomeMessage = true;
            if (firstTime) {
                gameWrapper->SetTimeout([this](GameWrapper* gw) {
                    if (!isWindowOpen_) {
                        cvarManager->executeCommand("togglemenu " + GetMenuName());
                    }
                }, 1.5f);
            }
        });


    gameWrapper->HookEventPost("Function TAGame.Replay_TA.StartPlaybackAtFrame", [this](std::string eventName) {
        isInReplay = true;
        snapshotManager.currentReplayState.captureSource = CaptureSource::Replay;
        });
    gameWrapper->HookEvent("Function TAGame.GameInfo_Replay_TA.Destroyed", [this](std::string eventName) {
        isInReplay = false;
        snapshotManager.currentReplayState.captureSource = CaptureSource::Unknown;
        });
    


    gameWrapper->HookEventWithCallerPost<CameraWrapper >(
        "Function TAGame.Camera_Replay_TA.GetFocusCar",
        [this](CameraWrapper  caller, void* params, std::string eventName) {
            std::string actorName = caller.GetFocusActor();
            if(actorName == "None" || actorName == "AutoCam" || actorName == "Ball" || actorName.empty()) return;
            size_t firstBar = actorName.find('|');
            size_t secondBar = actorName.find('|', firstBar + 1);
            if (secondBar != std::string::npos) {
                focusCarID = actorName.substr(firstBar + 1, secondBar - firstBar - 1);
            }
            //LOG("Focus actor name: {}", focusCarID);
        });

    gameWrapper->HookEventWithCallerPost<ActorWrapper>("Function GameEvent_TrainingEditor_TA.Countdown.Tick", [this](ActorWrapper cw, void* params, std::string eventName) {
        if (isInTrainingEditor()||isInTrainingPack()) {
            


            snapshotManager.currentReplayState.carVelocity = currentShotState.extendedStartingVelocity;
            snapshotManager.currentReplayState.carAngularVelocity = currentShotState.extendedStartingAngularVelocity;
            auto server = gameWrapper->GetCurrentGameState();
            auto car = server.GetGameCar();
            if (car.IsNull()) {
				LOG("Car is null");
				return;
			}
            BoostWrapper boost = car.GetBoostComponent();
            if (boost.IsNull()) {
                LOG("Boost component is null");
                return;
            }
            snapshotManager.currentReplayState.boostAmount = (boost.GetCurrentBoostAmount())*100.f;
            snapshotManager.currentReplayState.carRotation = car.GetRotation();
            snapshotManager.currentReplayState.carLocation = car.GetLocation();
            snapshotManager.currentReplayState.hasJump = !car.GetbJumped();


            currentShotState.carRotation = car.GetRotation();
            currentShotState.carLocation = car.GetLocation();
           
		}
		});
    gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.Ball_GameEditor_TA.EditorMoveToLocation", [this](ActorWrapper cw, void* params, std::string eventName) {
        if (!isInTrainingEditor())return;
        struct pExecEditorMoveToLocaction
        {
            struct Vector NewLocation;
          
        };
        auto* p = reinterpret_cast<pExecEditorMoveToLocaction*>(params);
        if (!savedReplayState.ballSet) {
            p->NewLocation.X = savedReplayState.ballLocation.X;
            p->NewLocation.Y = savedReplayState.ballLocation.Y;
            p->NewLocation.Z = savedReplayState.ballLocation.Z;
            snapshotManager.currentReplayState.ballLocation= p->NewLocation;
            return;
        } else if (lockScene) {
            auto server = gameWrapper->GetCurrentGameState();
            auto ball = server.GetBall();
            if (ball.IsNull()) {
				LOG("Ball is null");
				return;
			}
            
            p->NewLocation.X = ball.GetLocation().X;
            p->NewLocation.Y = ball.GetLocation().Y;
            p->NewLocation.Z = ball.GetLocation().Z;
            //p->NewLocation = snapshotManager.currentReplayState.ballLocation;
            return;
        }
        
        

        });

   

    //TAGame.Ball_GameEditor_TA.EventVelocityStartSpeedChanged
    gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.Ball_GameEditor_TA.Tick", [this](ActorWrapper cw, void* params, std::string eventName) {
        if (!(isInTrainingEditor()||isInTrainingPack()))return;
        struct cBall_GameEditor_TA {
            unsigned char pad[0x0AD0];
            Vector StartLocation;
            Rotator StartRotation;
            float VelocityStartSpeed;
            Rotator VelocityStartRotation;
        };
        cBall_GameEditor_TA* caller = (cBall_GameEditor_TA*)cw.memory_address;
        
        snapshotManager.currentReplayState.ballSpeed = caller->VelocityStartSpeed;
        snapshotManager.currentReplayState.ballLocation = caller->StartLocation;
        snapshotManager.currentReplayState.ballRotation = caller->VelocityStartRotation;
        
        //LOG("ball location from tick : {}, {}, {}", caller->StartLocation.X, caller->StartLocation.Y, caller->StartLocation.Z);

        if (!isInTrainingEditor())return;
        if (savedReplayState.ballSet) return;
        

        //caller->StartLocation = savedReplayState.ballLocation;
        caller->VelocityStartSpeed = savedReplayState.ballSpeed;
        caller->VelocityStartRotation = savedReplayState.ballRotation;
        currentShotState.extendedStartingVelocity = savedReplayState.carVelocity;
        currentShotState.startingVelocity = savedReplayState.carVelocity.magnitude();
        currentShotState.extendedStartingAngularVelocity = savedReplayState.carAngularVelocity;
        if(caller->StartLocation.X == savedReplayState.ballLocation.X && caller->StartLocation.Y == savedReplayState.ballLocation.Y && caller->StartLocation.Z == savedReplayState.ballLocation.Z){
            savedReplayState.ballSet = true;
        }
        



        });


    


    gameWrapper->HookEventWithCaller<BallWrapper>("Function TAGame.Ball_GameEditor_TA.AddVelocityStartRotation", [this](BallWrapper cw, void* params, std::string eventName) {
     
        //unsigned char pad[0x0010];
        struct pReplicationRotation {
        Rotator rot;
        };
        auto* p = reinterpret_cast<pReplicationRotation*>(params);
        
        if (lockScene) {
            p->rot = Rotator(0, 0, 0);
        }
       
        });

    //TAGame.Ball_GameEditor_TA.ModfiyBlendedRotation
    gameWrapper->HookEventWithCallerPost<ActorWrapper>("Function TAGame.Ball_GameEditor_TA.ModfiyBlendedRotation", [this](ActorWrapper cw, void* params, std::string eventName) {
        if (!isInTrainingEditor() )return;
        
        struct pReplicationRotation {
            unsigned char pad[0x0033];
            Rotator rot;
        };
        auto * p = reinterpret_cast<pReplicationRotation*>(params);

        currentRotationInTrainingEditor = p->rot;
        //LOG("rotation12: {}, {}, {}", p->rot.Pitch, p->rot.Yaw, p->rot.Roll);
        //p->rot.Pitch = 2000;
        });
}

void VersatileTraining::handleGetFocusCar() {

}


//cw.SetRotation(Rotator(8192, 0, 0));
        //gameWrapper->GetCamera().SetRotation(Rotator(8192, 0, 0));

        /*int32_t* paramsAsSignedInts = reinterpret_cast<int32_t*>(params);

        for (int i = 0; i < 15; i++) {
            LOG("Param {} (signed): {}", i, paramsAsSignedInts[i]);
        }*/
        /*LOG("Param 0 (signed): {}", paramsAsSignedInts[0]);
        LOG("Param 1 (signed): {}", paramsAsSignedInts[1]);
        LOG("Param 2 (signed): {}", paramsAsSignedInts[2]);
        LOG("Param 3 (signed): {}", paramsAsSignedInts[3]);*/

        /*LOG("setting rotation");
        LOG("rotation: {}, {}, {}", cw.GetRotation().Pitch, cw.GetRotation().Yaw, cw.GetRotation().Roll);
        cw.SetRotation(Rotator(8192, 0, 0));
        int numBytes = 256;
        uint8_t* bytes = static_cast<uint8_t*>(params);
        std::ostringstream oss;
        oss << "Params dump: ";
        for (size_t i = 0; i < numBytes; ++i) {
            oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(bytes[i]) << " ";
        }
        LOG("{}", oss.str());*/