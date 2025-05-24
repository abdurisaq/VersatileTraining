#include "pch.h"


CustomTrainingDataflattened CustomTrainingData::deflate() {
	CustomTrainingDataflattened flatData;
	flatData.code = code;
	flatData.name = name;
	flatData.numShots = shots.size();
	flatData.currentEditedShot = currentEditedShot;

	for (const auto& shot : shots) {
		flatData.boostAmounts.push_back(shot.boostAmount);
		flatData.freezeCar.push_back(shot.freezeCar);
		flatData.startingVelocity.push_back(shot.startingVelocity);
		flatData.goalBlockers.push_back(shot.goalBlocker);
		flatData.goalAnchors.push_back(shot.goalAnchors);
		flatData.hasStartingJump.push_back(shot.hasJump);
        flatData.extendedStartingVelocities.push_back(shot.extendedStartingVelocity);
        flatData.extendedStartingAngularVelocities.push_back(shot.extendedStartingAngularVelocity);
		
	}


	return flatData;
}

CustomTrainingData CustomTrainingDataflattened::inflate() {
    CustomTrainingData inflatedData;
    inflatedData.code = code;
    inflatedData.name = name;
    inflatedData.numShots = numShots;
    inflatedData.currentEditedShot = currentEditedShot;

    if (hasStartingJump.size() != numShots) {
        LOG("Warning: hasStartingJump size mismatch, resizing");
        hasStartingJump.resize(numShots, true); // Default to true
    }

    for (int i = 0; i < numShots; i++) {
        ShotState shot(
            i < freezeCar.size() ? freezeCar[i] : false,
            i < hasStartingJump.size() ? hasStartingJump[i] : true,
            i < startingVelocity.size() ? startingVelocity[i] : 0,
            i < boostAmounts.size() ? boostAmounts[i] : 101,
            i < goalBlockers.size() ? goalBlockers[i] : std::pair<Vector, Vector>{ {0,0,0},{0,0,0} },
            i < goalAnchors.size() ? goalAnchors[i] : std::pair<bool, bool>{ false, false },
            i < extendedStartingVelocities.size() ? extendedStartingVelocities[i] : Vector{ 0,0,0 },
            i < extendedStartingAngularVelocities.size() ? extendedStartingAngularVelocities[i] : Vector{ 0,0,0 },
            ShotRecording()
        );
        inflatedData.shots.push_back(shot);
    }

    return inflatedData;
}


