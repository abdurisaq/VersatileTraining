#pragma once
#include <pch.h>

class MovementUtils {

    int clampVal = 0;
    bool isCeiling = false;
    float diagBound = 7965;
    float currentXBound = 4026.0f;
    float currentYBound = 5050.0f;
    float t = 0.0f;
    float frozenZVal = 0.0f;
    bool frozeZVal = false;

    // Misc
    bool isCarRotatable = false;
    
public:
     Rotator checkForClamping(Vector loc, Rotator rot);
     Vector getClampChange(Vector loc, Rotator rot);
     std::pair<float, float> getAxisBreakDown(Rotator rot, int extra);
     Vector getStickingVelocity(Rotator rot);
     void applyLocalPitch(Rotator& localRotation, float pitchInput);
};