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
            if (actorName == "None" || actorName == "AutoCam" || actorName == "Ball" || actorName.empty()) return;
            size_t firstBar = actorName.find('|');
            size_t secondBar = actorName.find('|', firstBar + 1);
            if (secondBar != std::string::npos) {
                focusCarID = actorName.substr(firstBar + 1, secondBar - firstBar - 1);
            }
            // 
        });

    gameWrapper->HookEventWithCallerPost<ActorWrapper>("Function GameEvent_TrainingEditor_TA.Countdown.Tick", [this](ActorWrapper cw, void* params, std::string eventName) {
        if (isInTrainingEditor() || isInTrainingPack()) {



            snapshotManager.currentReplayState.carVelocity = currentShotState.extendedStartingVelocity;
            snapshotManager.currentReplayState.carAngularVelocity = currentShotState.extendedStartingAngularVelocity;
            auto server = gameWrapper->GetCurrentGameState();
            auto car = server.GetGameCar();
            if (car.IsNull()) {

                return;
            }
            BoostWrapper boost = car.GetBoostComponent();
            if (boost.IsNull()) {

                return;
            }
            snapshotManager.currentReplayState.boostAmount = (boost.GetCurrentBoostAmount()) * 100.f;
            snapshotManager.currentReplayState.carRotation = car.GetRotation();
            snapshotManager.currentReplayState.carLocation = car.GetLocation();
            snapshotManager.currentReplayState.hasJump = !car.GetbJumped();


            currentShotState.carRotation = car.GetRotation();
            currentShotState.carLocation = car.GetLocation();

        }
        });
    gameWrapper->HookEventWithCallerPost<ActorWrapper>("Function TAGame.Ball_GameEditor_TA.EditorMoveToLocation", [this](ActorWrapper cw, void* params, std::string eventName) {
        if (!isInTrainingEditor())return;
        struct cBall_GameEditor_TA {
            unsigned char pad[0xb10];
            Vector StartLocation;
            Rotator StartRotation;
            float VelocityStartSpeed;
            Rotator VelocityStartRotation;
        };
        cBall_GameEditor_TA* caller = (cBall_GameEditor_TA*)cw.memory_address;





        struct pExecEditorMoveToLocaction
        {
            struct Vector Loc; // 0x0000
            uint32_t ReturnValue; // 0x000c
        };

        auto* p = reinterpret_cast<pExecEditorMoveToLocaction*>(params);

        if (!savedReplayState.ballLocationSet) {

            float epsilon = 0.01f;
            if (abs(cw.GetLocation().X - savedReplayState.ballLocation.X < epsilon) &&
                abs(cw.GetLocation().Y - savedReplayState.ballLocation.Y < epsilon) &&
                abs(cw.GetLocation().Z - savedReplayState.ballLocation.Z < epsilon)) {
                savedReplayState.ballLocationSet = true;
            }

            else {
                LOG("Ball location not set, setting to saved location: {} {} {}", savedReplayState.ballLocation.X, savedReplayState.ballLocation.Y, savedReplayState.ballLocation.Z);
                LOG("Ball location: {} {} {}", cw.GetLocation().X, cw.GetLocation().Y, cw.GetLocation().Z);
                cw.SetLocation(savedReplayState.ballLocation);
            }


            return;
        }
        else
            if (lockScene) {
                auto server = gameWrapper->GetCurrentGameState();
                if (server.IsNull()) {
                    LOG("Server is null in EditorMoveToLocation");
                    return;
                }
                auto ball = server.GetBall();
                if (ball.IsNull()) {
                    LOG("Ball is null in EditorMoveToLocation");
                    return;
                }


                return;
            }
        });

    gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.Ball_GameEditor_TA.EditorMoveToLocation", [this](ActorWrapper cw, void* params, std::string eventName) {
        if (!isInTrainingEditor())return;


        struct pExecEditorMoveToLocaction
        {
            struct Vector Loc; // 0x0000
            uint32_t ReturnValue;
        };

        auto* p = reinterpret_cast<pExecEditorMoveToLocaction*>(params);

        if (lockScene) {
            auto server = gameWrapper->GetCurrentGameState();
            if (server.IsNull()) {
                LOG("Server is null in EditorMoveToLocation");
                return;
            }
            auto ball = server.GetBall();
            if (ball.IsNull()) {
                LOG("Ball is null in EditorMoveToLocation");
                return;
            }
            p->Loc = ball.GetLocation();
            p->ReturnValue = 1;

            return;
        }

        });




    //TAGame.Ball_GameEditor_TA.EventVelocityStartSpeedChanged


    gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.Ball_GameEditor_TA.Tick", [this](ActorWrapper cw, void* params, std::string eventName) {
        if (!(isInTrainingEditor() || isInTrainingPack()))return;
        struct cBall_GameEditor_TA {
            unsigned char pad[0xb10];
            Vector StartLocation;
            Rotator StartRotation;
            float VelocityStartSpeed;
            Rotator VelocityStartRotation;
        };
        cBall_GameEditor_TA* caller = (cBall_GameEditor_TA*)cw.memory_address;

        snapshotManager.currentReplayState.ballSpeed = caller->VelocityStartSpeed;
        snapshotManager.currentReplayState.ballLocation = caller->StartLocation;
        snapshotManager.currentReplayState.ballRotation = caller->VelocityStartRotation;



        if (!isInTrainingEditor())return;
        if (savedReplayState.ballRotationSet) return;


        caller->VelocityStartSpeed = savedReplayState.ballSpeed;
        caller->VelocityStartRotation = savedReplayState.ballRotation;

        currentShotState.extendedStartingVelocity = savedReplayState.carVelocity;
        currentShotState.startingVelocity = savedReplayState.carVelocity.magnitude();
        currentShotState.extendedStartingAngularVelocity = savedReplayState.carAngularVelocity;

        savedReplayState.ballRotationSet = true;






        });





    gameWrapper->HookEventWithCaller<BallWrapper>("Function TAGame.Ball_GameEditor_TA.AddVelocityStartRotation", [this](BallWrapper cw, void* params, std::string eventName) {

        //savedReplayState.ballRotation
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
        if (!isInTrainingEditor())return;

        struct pReplicationRotation {
            unsigned char pad[0x0033];
            Rotator rot;
        };
        auto* p = reinterpret_cast<pReplicationRotation*>(params);

        currentRotationInTrainingEditor = p->rot;
        // 
        //p->rot.Pitch = 2000;
        });
}

void VersatileTraining::handleGetFocusCar() {

}


//cw.SetRotation(Rotator(8192, 0, 0));
        //gameWrapper->GetCamera().SetRotation(Rotator(8192, 0, 0));

        /*int32_t* paramsAsSignedInts = reinterpret_cast<int32_t*>(params);

        for (int i = 0; i < 15; i++) {

        }*/
        /*


         */

         /*

         cw.SetRotation(Rotator(8192, 0, 0));
         int numBytes = 256;
         uint8_t* bytes = static_cast<uint8_t*>(params);
         std::ostringstream oss;
         oss << "Params dump: ";
         for (size_t i = 0; i < numBytes; ++i) {
             oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(bytes[i]) << " ";
         }
          */