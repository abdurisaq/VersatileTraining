#include "pch.h"
#include "src/core/versatileTraining.h"

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

    //add when start play test is called, to save goalblocker for the currently edited shot
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
    if (!isInTrainingEditor())return;
    if (editingGoalBlocker) {
        Rotator rot = gameWrapper->GetCamera().GetRotation();
        Vector loc = gameWrapper->GetCamera().GetLocation();
        bool rotChanged = false;

        if (!savedPastCamera) {
            previousLocBeforeGoalEdit = loc;
            previousRotBeforeGoalEdit = rot;
            savedPastCamera = true;
        }

        /* 
         */

        loc.Y = 4000.f;
        loc.X = 0.f;
        loc.Z = 321.f;

        float centerYaw = 16384.f;
        float maxYawRange = 7000.f;
        float normalizedYawDist = std::abs(rot.Yaw - centerYaw) / maxYawRange;
        normalizedYawDist = std::clamp(normalizedYawDist, 0.f, 1.f);

        float extraPitch = (1.f - normalizedYawDist) * 600.f;
        int allowedPitch = (int)(2400.f + extraPitch);

        if (rot.Pitch > allowedPitch) {
            rot.Pitch = allowedPitch;
            rotChanged = true;
        }
        else if (rot.Pitch < -allowedPitch) {
            rot.Pitch = -allowedPitch;
            rotChanged = true;
        }

        if (rot.Yaw < 9384) {
            rot.Yaw = 9384;
            rotChanged = true;
        }
        else if (rot.Yaw > 23384) {
            rot.Yaw = 23384;
            rotChanged = true;
        }

        gameWrapper->GetCamera().SetLocation(loc);
        if (rotChanged) {
            gameWrapper->GetCamera().SetRotation(rot);
        }
    }
    else if (!editingGoalBlocker && savedPastCamera) {
         
        gameWrapper->GetCamera().SetLocation(previousLocBeforeGoalEdit);
        gameWrapper->GetCamera().SetRotation(previousRotBeforeGoalEdit);
        previousLocBeforeGoalEdit = { 0, 0, 0 };
        previousRotBeforeGoalEdit = { 0, 0, 0 };
        savedPastCamera = false;
    }
    
    
}

