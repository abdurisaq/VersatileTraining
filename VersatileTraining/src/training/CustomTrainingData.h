#pragma  once
#include <pch.h>

struct CustomTrainingData;
struct ShotState {
	int shotIndex = 0;
	bool freezeCar;
	bool hasJump;
	int startingVelocity;
	Vector extendedStartingVelocity;
	Vector extendedStartingAngularVelocity;
	int boostAmount;
	std::pair<Vector, Vector> goalBlocker;
	std::pair<bool, bool> goalAnchors;

	ShotRecording recording;
	Vector carLocation = Vector(0,0,0);
	Rotator carRotation = Rotator(0,0,0);
	ShotState() : freezeCar(false), hasJump(true), startingVelocity(0), boostAmount(101), goalBlocker({ { 0, 0, 0 }, { 0, 0, 0 } }), goalAnchors({ false, false }), carLocation({ 0,0,0 }), carRotation({0,0,0}), extendedStartingVelocity({ 0,0,0 }),extendedStartingAngularVelocity({0,0,0}), recording(ShotRecording()) {}
	ShotState(bool freeze, bool jump, int velocity, int boost, std::pair<Vector, Vector> blocker, std::pair<bool, bool> anchors,  Vector startingVelocity, Vector angularVelocity,ShotRecording recording)
		: freezeCar(freeze), hasJump(jump), startingVelocity(velocity), boostAmount(boost), goalBlocker(blocker), goalAnchors(anchors), extendedStartingVelocity(startingVelocity), extendedStartingAngularVelocity(angularVelocity), recording(recording) {}

	ShotState& operator=(const ShotState& other) {
		if(&other == nullptr) return *this; 
		if (this != &other) {
			freezeCar = other.freezeCar;
			hasJump = other.hasJump;
			startingVelocity = other.startingVelocity;
			extendedStartingVelocity = other.extendedStartingVelocity;
			extendedStartingAngularVelocity = other.extendedStartingAngularVelocity;
			boostAmount = other.boostAmount;
			goalBlocker = other.goalBlocker;
			goalAnchors = other.goalAnchors;
			carLocation = other.carLocation;
			carRotation = other.carRotation;

			
			try {
				if (&other.recording == nullptr) {
					recording = ShotRecording();
				}
				else if (!other.recording.inputs.data()) { 
					
					recording = ShotRecording();
					recording.carBody = other.recording.carBody;
					recording.settings = other.recording.settings;
					recording.initialState = other.recording.initialState;
					recording.startState = other.recording.startState;
					
				}
				else if (other.recording.inputs.size() > 100000) {
					
					recording = ShotRecording();
					recording.carBody = other.recording.carBody;
					recording.settings = other.recording.settings;
					recording.initialState = other.recording.initialState;
					recording.startState = other.recording.startState;
					
				}
				else {
					
					ShotRecording newRecording;
					newRecording.carBody = other.recording.carBody;
					newRecording.settings = other.recording.settings;
					newRecording.initialState = other.recording.initialState;
					newRecording.startState = other.recording.startState;

					try {
						if (other.recording.inputs.size() > 0) {
							newRecording.inputs.reserve(other.recording.inputs.size());
							for (const auto& input : other.recording.inputs) {
								newRecording.inputs.push_back(input);
							}
						}
					}
					catch (const std::exception& e) {
						LOG("Exception copying inputs: {}", e.what());
						
					}

					recording = std::move(newRecording);
				}
			}
			catch (const std::exception& e) {
				
				LOG("Exception in ShotState assignment: {}", e.what());
				recording = ShotRecording();
			}
		}
		return *this;
	}
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
	std::vector<Vector> extendedStartingAngularVelocities;
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
		extendedStartingAngularVelocities = std::vector<Vector>(shotAmount, { 0,0,0 });
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
	int maxVelocity = 2300;
	int minVelocity = -2300;
	void initCustomTrainingData(int shotAmount, const std::string& packName, const std::string & packCode) {
		LOG("called initCustomTrainingData");
		name = packName;
		code = packCode;
		numShots = shotAmount;
		currentEditedShot = 0;
		shots = std::vector<ShotState>(shotAmount); 
	}

	void addShot(const ShotState& state = ShotState()) {
		numShots++;
		shots.push_back(state);
		shots.back().shotIndex = numShots - 1;
	}
	void reset() {
		numShots = 0;
		currentEditedShot = -1;
		shots.clear();
	}
	CustomTrainingDataflattened deflate();

	std::string compressAndEncodeTrainingData();
};




struct PackOverrideSettings {
	// Flags to indicate if the setting is overridden
	bool overrideBoostLimit = false;
	bool overridePlayerStartVelocity = false;
	bool overrideBallSpeedVariancePct = false;
	bool overrideTrajectoryRotationVariancePct = false;
	bool overrideBallLocationVarianceXY = false;
	bool overrideBallLocationVarianceHeight = false;
	bool overrideBallSpin = false;
	bool overrideTrainingTimeLimit = false;
	bool overrideCarLocationVarianceXY = false;
	bool overrideCarRotationVariancePct = false;

	// Actual override values
	int boostLimit = -1;
	int playerStartVelocity[2] = { 0, 2000 };
	float ballSpeedVariancePct[2] = { -25.0f, 25.0f };
	float trajectoryRotationVariancePct[2] = { -25.0f, 25.0f };
	float ballLocationVarianceXY[2] = { -250.0f, 250.0f };
	float ballLocationVarianceHeight[2] = { -1000.0f, 1000.0f };
	float ballSpin[2] = { -6.0f, 6.0f };
	int trainingTimeLimit = 0;
	float carLocationVarianceXY[2] = { -500.0f, 500.0f };
	float carRotationVariancePct[2] = { -50.0f, 50.0f };

	PackOverrideSettings() = default;

	bool HasAnyOverride() const {
		return overrideBoostLimit || overridePlayerStartVelocity || overrideBallSpeedVariancePct ||
			overrideTrajectoryRotationVariancePct || overrideBallLocationVarianceXY ||
			overrideBallLocationVarianceHeight || overrideBallSpin || overrideTrainingTimeLimit ||
			overrideCarLocationVarianceXY || overrideCarRotationVariancePct;
	}
	void ApplyCVars(std::shared_ptr<CVarManagerWrapper> cvarManager, bool logCommands = false) const {
		// Buffer for command strings
		char cmdBuffer[128];

		// Enable training overrides if any overrides are active
		if (HasAnyOverride()) {
			cvarManager->executeCommand("sv_training_enabled 1", false);
		}
		else {
			cvarManager->executeCommand("sv_training_enabled 0", false);
		}

		


		int boostToSet = overrideBoostLimit ? boostLimit : -1;
		sprintf(cmdBuffer, "sv_training_limitboost %d", boostToSet);
		cvarManager->executeCommand(cmdBuffer, false);
		if (logCommands) { LOG("Executed CVar: %s", cmdBuffer); }

		int velMin = overridePlayerStartVelocity ? playerStartVelocity[0] : 0;
		int velMax = overridePlayerStartVelocity ? playerStartVelocity[1] : 0;
		sprintf(cmdBuffer, "sv_training_player_velocity (%d, %d)", velMin, velMax);
		cvarManager->executeCommand(cmdBuffer, false);
		if (logCommands) { LOG("Executed CVar: %s", cmdBuffer); }

		float speedVarMin = overrideBallSpeedVariancePct ? ballSpeedVariancePct[0] : 0.0f;
		float speedVarMax = overrideBallSpeedVariancePct ? ballSpeedVariancePct[1] : 0.0f;
		sprintf(cmdBuffer, "sv_training_var_speed (%.1f, %.1f)", speedVarMin, speedVarMax);
		cvarManager->executeCommand(cmdBuffer, false);
		if (logCommands) { LOG("Executed CVar: %s", cmdBuffer); }

		float trajRotVarMin = overrideTrajectoryRotationVariancePct ? trajectoryRotationVariancePct[0] : 0.0f;
		float trajRotVarMax = overrideTrajectoryRotationVariancePct ? trajectoryRotationVariancePct[1] : 0.0f;
		sprintf(cmdBuffer, "sv_training_var_rot (%.1f, %.1f)", trajRotVarMin, trajRotVarMax);
		cvarManager->executeCommand(cmdBuffer, false);
		if (logCommands) { LOG("Executed CVar: %s", cmdBuffer); }

		float ballLocXYMin = overrideBallLocationVarianceXY ? ballLocationVarianceXY[0] : 0.0f;
		float ballLocXYMax = overrideBallLocationVarianceXY ? ballLocationVarianceXY[1] : 0.0f;
		sprintf(cmdBuffer, "sv_training_var_loc (%.1f, %.1f)", ballLocXYMin, ballLocXYMax);
		cvarManager->executeCommand(cmdBuffer, false);
		if (logCommands) { LOG("Executed CVar: %s", cmdBuffer); }

		float ballLocZMin = overrideBallLocationVarianceHeight ? ballLocationVarianceHeight[0] : 0.0f;
		float ballLocZMax = overrideBallLocationVarianceHeight ? ballLocationVarianceHeight[1] : 0.0f;
		sprintf(cmdBuffer, "sv_training_var_loc_z (%.1f, %.1f)", ballLocZMin, ballLocZMax);
		cvarManager->executeCommand(cmdBuffer, false);
		if (logCommands) { LOG("Executed CVar: %s", cmdBuffer); }

		float ballSpinMin = overrideBallSpin ? ballSpin[0] : 0.0f;
		float ballSpinMax = overrideBallSpin ? ballSpin[1] : 0.0f;
		sprintf(cmdBuffer, "sv_training_var_spin (%.1f, %.1f)", ballSpinMin, ballSpinMax);
		cvarManager->executeCommand(cmdBuffer, false);
		if (logCommands) { LOG("Executed CVar: %s", cmdBuffer); }

		float carLocXYMin = overrideCarLocationVarianceXY ? carLocationVarianceXY[0] : 0.0f;
		float carLocXYMax = overrideCarLocationVarianceXY ? carLocationVarianceXY[1] : 0.0f;
		sprintf(cmdBuffer, "sv_training_var_car_loc (%.1f, %.1f)", carLocXYMin, carLocXYMax);
		cvarManager->executeCommand(cmdBuffer, false);
		if (logCommands) { LOG("Executed CVar: %s", cmdBuffer); }

		float carRotVarMin = overrideCarRotationVariancePct ? carRotationVariancePct[0] : 0.0f;
		float carRotVarMax = overrideCarRotationVariancePct ? carRotationVariancePct[1] : 0.0f;
		sprintf(cmdBuffer, "sv_training_var_car_rot (%.1f, %.1f)", carRotVarMin, carRotVarMax);
		cvarManager->executeCommand(cmdBuffer, false);
		if (logCommands) { LOG("Executed CVar: %s", cmdBuffer); }

	}

};