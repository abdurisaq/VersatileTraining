#include "pch.h"


CustomTrainingDataflattened CustomTrainingData::deflate() {
	CustomTrainingDataflattened flatData;
	flatData.code = code;
	flatData.name = name;
	flatData.numShots = numShots;
	flatData.currentEditedShot = currentEditedShot;

	for (const auto& shot : shots) {
		flatData.boostAmounts.push_back(shot.boostAmount);
		flatData.freezeCar.push_back(shot.freezeCar);
		flatData.startingVelocity.push_back(shot.startingVelocity);
		flatData.goalBlockers.push_back(shot.goalBlocker);
		flatData.goalAnchors.push_back(shot.goalAnchors);
		flatData.hasStartingJump.push_back(shot.hasJump);
	}

	return flatData;
}

CustomTrainingData CustomTrainingDataflattened::inflate() {
	CustomTrainingData	 inflatedData;
	inflatedData.code = code;
	inflatedData.name = name;
	inflatedData.numShots = numShots;
	inflatedData.currentEditedShot = currentEditedShot;

	for (int i = 0; i < numShots; i++) {
		inflatedData.shots.push_back(ShotState(freezeCar[i], hasStartingJump[i], startingVelocity[i], boostAmounts[i], goalBlockers[i], goalAnchors[i]));
	}
	return inflatedData;
}