#pragma once
#include <pch.h>

struct CustomTrainingData;
struct ShotState {
	bool freezeCar;
	bool hasJump;
	int startingVelocity;
	Vector extendedStartingVelocity;
	Vector extendedStartingAngularVelocity;
	int boostAmount;
	std::pair<Vector, Vector> goalBlocker;
	std::pair<bool, bool> goalAnchors;

	Vector carLocation = Vector(0,0,0);
	Rotator carRotation = Rotator(0,0,0);
	ShotState() : freezeCar(false), hasJump(true), startingVelocity(0), boostAmount(101), goalBlocker({ { 0, 0, 0 }, { 0, 0, 0 } }), goalAnchors({ false, false }), carLocation({ 0,0,0 }), carRotation({0,0,0}), extendedStartingVelocity({ 0,0,0 }) {}
	ShotState(bool freeze, bool jump, int velocity, int boost, std::pair<Vector, Vector> blocker, std::pair<bool, bool> anchors,  Vector startingVelocity)
		: freezeCar(freeze), hasJump(jump), startingVelocity(velocity), boostAmount(boost), goalBlocker(blocker), goalAnchors(anchors), extendedStartingVelocity(startingVelocity) {}

};

struct CustomTrainingDataflattened {
	std::string code;
	std::string name;
	int numShots = 0;
	int currentEditedShot = 0;
	std::vector<int> boostAmounts;
	std::vector<bool> freezeCar;
	std::vector<int> startingVelocity;
	std::vector<std::pair<Vector, Vector>> goalBlockers;
	std::vector<std::pair<bool, bool>>goalAnchors;
	std::vector<bool> hasStartingJump;
	std::vector<Vector> extendedStartingVelocities;
	bool customPack = false;

	void initCustomTrainingData(int shotAmount, std::string packName) {
		LOG("called initCustomTrainingData");
		name = packName;
		numShots = shotAmount;
		currentEditedShot = 0;
		boostAmounts = std::vector<int>(shotAmount, 101);
		startingVelocity = std::vector<int>(shotAmount, 0);
		freezeCar = std::vector<bool>(shotAmount, false);
		goalBlockers = std::vector<std::pair<Vector, Vector>>(shotAmount, { { 0, 0, 0 }, { 0, 0, 0 } });
		goalAnchors = std::vector<std::pair<bool, bool>>(shotAmount, { false, false });
		hasStartingJump = std::vector<bool>(shotAmount, true);
		extendedStartingVelocities = std::vector<Vector>(shotAmount, { 0,0,0 });
	}
	void addShot(int boostAmount = 101, int velocity = 0, bool frozen = false, Vector firstAnchor = { 0,0,0 }, Vector secondAnchor = { 0,0,0 }) {
		numShots++;
		boostAmounts.push_back(boostAmount);
		startingVelocity.push_back(velocity);
		freezeCar.push_back(frozen);
		goalBlockers.push_back({ firstAnchor, secondAnchor });
		if (firstAnchor.X == 0 && firstAnchor.Z == 0 && secondAnchor.X == 0 && secondAnchor.Z == 0) {
			goalAnchors.push_back({ false, false });
			LOG("in add shot, first and second anchor are 0");
		}
		else {
			goalAnchors.push_back({ true, true });
			LOG("in add shot, first and second anchor are not 0");
		}
	}
	void reset() {
		numShots = 0;
		currentEditedShot = -1;
		boostAmounts.clear();
		startingVelocity.clear();
		freezeCar.clear();
		goalBlockers.clear();
		goalAnchors.clear();
		hasStartingJump.clear();
		extendedStartingVelocities.clear();
	}
	CustomTrainingData inflate();
};


struct CustomTrainingData {

	std::string code;
	std::string name;
	int numShots = 0;
	int currentEditedShot = 0;
	std::vector<ShotState> shots;
	bool customPack = false;

	int boostMax = 101;
	int boostMin = 0;
	int maxVelocity = 2000;
	int minVelocity = -2000;
	void initCustomTrainingData(int shotAmount, const std::string& packName) {
		LOG("called initCustomTrainingData");
		name = packName;
		numShots = shotAmount;
		currentEditedShot = 0;
		shots = std::vector<ShotState>(shotAmount);  // Default-constructed
	}

	void addShot(const ShotState& state = ShotState()) {
		numShots++;
		shots.push_back(state);
	}
	void reset() {
		numShots = 0;
		currentEditedShot = -1;
		shots.clear();
	}
	CustomTrainingDataflattened deflate();
};


