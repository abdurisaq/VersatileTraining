#include "pch.h"
#include "src/core/versatileTraining.h"

void VersatileTraining::replayHooks() {

    struct GetFocusCarParams {
        uintptr_t ReturnValue;
    };

    //TAGame.GameObserver_TA.UpdateBallData
    // TAGame.GameObserver_TA.UpdateCarsData
    // TAGame.Camera_Replay_TA.GetFocusCar
    //Function TAGame.ReplayManager_TA.PlayReplay
    gameWrapper->HookEventPost("Function TAGame.Replay_TA.StartPlaybackAtFrame", [this](std::string eventName) {
        isInReplay = true;
        });
    gameWrapper->HookEvent("Function TAGame.GameInfo_Replay_TA.Destroyed", [this](std::string eventName) {
        isInReplay = false;
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


//TAGame.GameEditor_Actor_TA.EditorMoveToLocation
//TAGame.GameEditor_Actor_TA.EditorSetRotation
// 
// 
// 
//Function TAGame.Ball_GameEditor_TA.EditorMoveToLocation
//Function TAGame.Ball_GameEditor_TA.EditorSetRotation
//Function Engine.PrimitiveComponent.SetRBLinearVelocity
//Function Engine.PrimitiveComponent.SetRBAngularVelocity

    gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.Ball_GameEditor_TA.EditorMoveToLocation", [this](ActorWrapper cw, void* params, std::string eventName) {
        if (!isInTrainingEditor())return;
        struct pExecEditorMoveToLocaction
        {
            struct Vector NewLocation;
          
        };
        if (!savedReplayState.filled) return;
        
        auto* p = reinterpret_cast<pExecEditorMoveToLocaction*>(params);
       
        p->NewLocation.X = savedReplayState.ballLocation.X;
        p->NewLocation.Y = savedReplayState.ballLocation.Y;
        p->NewLocation.Z = savedReplayState.ballLocation.Z;
        

        });

   

    //TAGame.Ball_GameEditor_TA.EventVelocityStartSpeedChanged
    gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.Ball_GameEditor_TA.Tick", [this](ActorWrapper cw, void* params, std::string eventName) {
        if (!isInTrainingEditor())return;
        struct cBall_GameEditor_TA {
            unsigned char pad[0x0AD0];
            Vector StartLocation;
            Rotator StartRotation;
            float VelocityStartSpeed;
            Rotator VelocityStratRotation;
        };
        cBall_GameEditor_TA* caller = (cBall_GameEditor_TA*)cw.memory_address;
        if (!savedReplayState.filled) return;
        
        caller->VelocityStartSpeed = savedReplayState.getBallShotFromVelocity().second;
        caller->VelocityStratRotation = savedReplayState.getBallShotFromVelocity().first;
        currentShotState.extendedStartingVelocity = savedReplayState.carVelocity;
        
        //LOG("speed: {}", caller->VelocityStartSpeed);
        /*LOG("ball location from tick : {}, {}, {}", caller->StartLocation.X, caller->StartLocation.Y, caller->StartLocation.Z);
        LOG("ball velocity from tick : {}, {}, {}", caller->VelocityStratRotation.Pitch, caller->VelocityStratRotation.Yaw, caller->VelocityStratRotation.Roll);
        LOG("ball speed from tick : {}", caller->VelocityStartSpeed);
        LOG("ball rotation from tick : {}, {}, {}", caller->StartRotation.Pitch, caller->StartRotation.Yaw, caller->StartRotation.Roll);*/
       //this works in other training packs, stuff can be captured this way to get the ball start strength and rotation



        });


    


    gameWrapper->HookEventWithCaller<BallWrapper>("Function TAGame.Ball_GameEditor_TA.AddVelocityStartRotation", [this](BallWrapper cw, void* params, std::string eventName) {
     
        //unsigned char pad[0x0010];
        struct pReplicationRotation {
        Rotator rot;
        };
        auto* p = reinterpret_cast<pReplicationRotation*>(params);
        //LOG("rotation: {}, {}, {}", p->rot.Pitch, p->rot.Yaw, p->rot.Roll);
 
        Rotator newRot = savedReplayState.getBallShotFromVelocity().first;
        //LOG("new rot : {}, {}, {}", newRot.Pitch, newRot.Yaw, newRot.Roll);


         // this is the current rotation the game will use

        const int32_t maxYawStep = 100;
        const int32_t maxPitchStep = 50;
        const int32_t fullCircle = 65536;

        int32_t deltaYaw = ((newRot.Yaw - currentRotationInTrainingEditor.Yaw + fullCircle / 2) % fullCircle) - fullCircle / 2;
        //LOG("delta yaw: {}", deltaYaw);
        deltaYaw = std::clamp(deltaYaw, -maxYawStep, maxYawStep);

        int32_t deltaPitch = newRot.Pitch - currentRotationInTrainingEditor.Pitch;
        deltaPitch = std::clamp(deltaPitch, -maxPitchStep, maxPitchStep);

        // Add delta to current value
        /*p->rot.Yaw = deltaYaw;
        p->rot.Pitch = deltaPitch;
        p->rot.Roll = 0;*/

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