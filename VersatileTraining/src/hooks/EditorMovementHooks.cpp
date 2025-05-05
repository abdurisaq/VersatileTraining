#include "pch.h"
#include "src/core/versatileTraining.h"

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


void VersatileTraining::handleUpdateCarData(ActorWrapper cw) {
    if (!currentTrainingData.customPack) {
        return;
    }

    cw.SetbCollideWorld(0);

    if (!(isInTrainingEditor() || isInTrainingPack())) return;

    ServerWrapper server = gameWrapper->GetCurrentGameState();
    if (!server) return;

    CarWrapper car = server.GetGameCar();
    if (!car) return;

    Rotator rot = car.GetRotation();
    Vector loc = car.GetLocation();

    if (!appliedWallClamping) {
        LOG("applying clamping to wall");
        Vector loc2 = getClampChange(loc, rot);
        car.SetLocation(loc2);
        appliedWallClamping = true;
    }
    if (!currentShotState.hasJump&& !appliedJumpState) {
        
        car.SetbJumped(true);
        car.SetbDoubleJumped(true);
        appliedJumpState = true;
    }
    
    
    if (freezeForShot) {
        handleFreezeCar(car, loc, rot);
    }
    else if (!freezeForShot && !appliedStartingVelocity) {
        handleUnfrozenCar(car, loc, rot);
    }
}

void VersatileTraining::handleStartRound() {

    LOG("startround called");

    if (isInTrainingEditor() || isInTrainingPack()) {
        shotReplicationManager.testCalledInStartRound = true;



        //if (!currentTrainingData.customPack) return;
        freezeForShot = false;
        frozeZVal = false;
        appliedStartingVelocity = false;

        ServerWrapper server = gameWrapper->GetCurrentGameState();
        if (!server) {
            			LOG("server not found");
			return;
        }
        ActorWrapper car = server.GetGameCar();
        if (!car) {

            		LOG("car not found");
			return;
        }


        if (currentShotState.extendedStartingVelocity != Vector(0,0,0)) {
            LOG("applying saved replay state in start round");
            car.SetVelocity(currentShotState.extendedStartingVelocity);
            car.SetAngularVelocity(currentShotState.extendedStartingAngularVelocity, 0);
            return;
        }
        Rotator rot = car.GetRotation();

        float pitchRad = (float)((rot.Pitch / 16201.0f) * (PI / 2));
        float yawRad = (float)((rot.Yaw / 32768.0f) * PI);
        float z = sinf(pitchRad);
        float y = cosf(pitchRad) * sinf(yawRad);
        float x = cosf(pitchRad) * cosf(yawRad);
        Vector unitVector = { x, y, z };
        //currentTrainingData.shots[currentTrainingData.currentEditedShot].startingVelocity
        int velocity = currentShotState.startingVelocity;
        if (velocity == 0) return;
        startingVelocityTranslation = unitVector * (float)velocity;
        Vector stickingVelocity = getStickingVelocity(rot);
        car.SetVelocity(startingVelocityTranslation + stickingVelocity);

        LOG("Calculated new velocity in StartRound");
    }
    else {
        LOG("not in training editor or pack");
    }
    
}


void VersatileTraining::handleEditorMoveToLocation(ActorWrapper cw, void* params) {
    if (!isInTrainingEditor())return;

    
    struct pExecEditorMoveToLocaction
    {
        struct Vector NewLocation;
        uint32_t ReturnValue : 1;
    };
    auto* p = reinterpret_cast<pExecEditorMoveToLocaction*>(params);

    if (!savedReplayState.carLocationSet) {
        p->NewLocation = savedReplayState.carLocation;
        currentShotState.carLocation = savedReplayState.carLocation;
        currentShotState.boostAmount = savedReplayState.boostAmount;
        currentShotState.freezeCar = true;
        savedReplayState.carLocationSet = true;
        return;
    }
    
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
    else {
        t = 0.f;
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


    else if (lockScene) {

        p->NewLocation = currentShotState.carLocation;
        currentShotState.boostAmount = savedReplayState.boostAmount;
        currentShotState.freezeCar = true;
        return;
    }

    // apply bounding
    p->NewLocation.X = std::clamp(p->NewLocation.X, -currentXBound, currentXBound);
    p->NewLocation.Y = std::clamp(p->NewLocation.Y, -currentYBound, currentYBound);
}


void VersatileTraining::handleEditorSetRotation(ActorWrapper cw) {
    if (!isInTrainingEditor())return;
    
    if (!savedReplayState.carRotationSet) {
        cw.SetRotation(savedReplayState.carRotation);
        currentShotState.carRotation = savedReplayState.carRotation;
        savedReplayState.carRotationSet = true;
        lockRotation = true;
    }

    Vector loc = cw.GetLocation();
    Rotator rot = cw.GetRotation();

    if (lockScene) {
        cw.SetRotation(currentShotState.carRotation);
        checkForClamping(loc, rot);
		lockRotation = true;
		return;
    }

    

    currentShotState.carRotation = rot;
    currentShotState.carLocation = loc;
    if (editingVariances && !lockRotation) {
        if (!cw || cw.IsNull()) {
            LOG("Server not found");
            return;
        }
        
        

        //LOG("rotating car");
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

            LOG("applying clamped roatation to car: {}, {}, {}", rot1.Pitch, rot1.Yaw, rot1.Roll);
            cw.SetRotation({ rot1.Pitch,rot1.Yaw,rot1.Roll });
        }

    }
    else if (editingVariances) {

        Rotator rot1 = checkForClamping(loc, rot);
        if (rot1.Yaw != 0 && rot.Pitch != 0 && rot.Roll != 0) {

            LOG("applying clamped roatation to car: {}, {}, {}", rot1.Pitch, rot1.Yaw, rot1.Roll);
            cw.SetRotation({ rot1.Pitch,rot1.Yaw,rot1.Roll });
        }
    }
}

void VersatileTraining::handleGetRotateActorCameraOffset(ActorWrapper cw) {
    if (!isInTrainingEditor())return;
    if (editingVariances && !lockRotation) {
        if (!cw || cw.IsNull()) {
            LOG("Server not found");
            return;
        }

        if (clampVal != 5) {
            //LOG("rotating car based on camera");
            Rotator rot = gameWrapper->GetCamera().GetRotation();
            rotationToApply.Pitch = rot.Pitch;
        }
    }
}
