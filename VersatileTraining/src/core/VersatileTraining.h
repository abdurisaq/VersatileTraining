#pragma once

#include "pch.h"
#include "include/GuiBase.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"
#include "bakkesmod/plugin/bakkesmodsdk.h"
#include "version.h"
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);


struct ReplayState {

	std::string replayName;

	std::string formattedTimeStampOfSaved;
	std::string replayTime;
	std::string timeRemainingInGame;

	std::string focusPlayerName;
	

	Vector carVelocity;
	Vector carAngularVelocity;
	Vector carLocation;
	Rotator carRotation;

	float focusPlayerBoostAmount;
	float jumpTimer;
	bool hasJump;
	bool boosting;

	Vector ballVelocity;
	Vector ballLocation;
	Rotator ballRotation;
	Vector ballAngularVelocity;

	bool filled = false;
	std::pair<Rotator, float> getBallShotFromVelocity() const {
		float vx = ballVelocity.X;
		float vy = ballVelocity.Y;
		float vz = ballVelocity.Z;

		float speed = sqrtf(vx * vx + vy * vy + vz * vz);

		if (speed == 0.0f) {
			return { Rotator{0, 0, 0}, 0.0f };
		}

		float nx = vx / speed;
		float ny = vy / speed;
		float nz = vz / speed;

		// Calculate angles in degrees
		float pitch_deg = std::asin(nz) * (180.0f / static_cast<float>(PI));
		float yaw_deg = std::atan2(ny, nx) * (180.0f / static_cast<float>(PI));

		// Normalize yaw_deg to [0, 360)
		if (yaw_deg < 0.0f) {
			yaw_deg += 360.0f;
		}

		// Convert to Unreal units
		// Pitch: -90° to +90° maps to -16384 to +16384
		int32_t pitch_units = static_cast<int32_t>(pitch_deg * (16384.0f / 90.0f));

		// Yaw: 0° to 360° maps to 0 to 65536
		int32_t yaw_units = static_cast<int32_t>(yaw_deg * (65536.0f / 360.0f));

		// Clamp pitch (though the calculation should already be in range)
		pitch_units = max(-16384, min(16384, pitch_units));

		Rotator rot{ pitch_units, yaw_units, 0 };
		float clampedSpeed = min(speed, 6000.0f);

		LOG("desired rotation {} {} {} " , rot.Pitch, rot.Yaw, rot.Roll);
		LOG("desired speed {}", clampedSpeed);
		return { rot, clampedSpeed };

	}

};






template <typename T, typename std::enable_if<std::is_base_of<ObjectWrapper, T>::value>::type*>
void GameWrapper::HookEventWithCallerPost(std::string eventtName, std::function<void(T caller, void* params, std::string eventName)> callback)
{
	auto wrapped_callback = [callback](ActorWrapper caller, void* params, std::string eventName) {
		callback(T(caller.memory_address), params, eventName);

		};
	HookEventWithCaller<ActorWrapper>(eventtName, wrapped_callback);

}


template <typename T, typename std::enable_if<std::is_base_of<ObjectWrapper, T>::value>::type*>
void GameWrapper::HookEventWithCaller(std::string eventName,
	std::function<void(T caller, void* params, std::string eventName)> callback)
{
	auto wrapped_callback = [callback](ActorWrapper caller, void* params, std::string eventName)
		{
			callback(T(caller.memory_address), params, eventName);
		};
	HookEventWithCaller<ActorWrapper>(eventName, wrapped_callback);
}
class VersatileTraining : public BakkesMod::Plugin::BakkesModPlugin
	, public SettingsWindowBase // Uncomment if you wanna render your own tab in the settings menu
	//,public PluginWindowBase // Uncomment if you want to render your own plugin window
{
	ShotReplicationManager shotReplicationManager;
	
	ShotState currentShotState;

	
	std::filesystem::path saveFilePath;


	std::unordered_map<std::string, CustomTrainingData> trainingData;
	std::string editingTrainingCode;
	bool editMode = false;

	bool editingGoalBlocker = false;
	bool goalBlockerEligbleToBeEdited = false;
	bool savedPastCamera = false;
	Vector previousLocBeforeGoalEdit = { 0, 0, 0 };
	Rotator previousRotBeforeGoalEdit = { 0, 0, 0 };

	CustomTrainingData currentTrainingData;
	

	std::string currentPackKey;
	int currentShotIndex = 0;

	
	
	int activeStartingVelocity = 0;

	bool editingVariances = false;
	bool appliedStartingVelocity = false;
	bool appliedWallClamping = false;
	bool appliedJumpState = false;

	// Rotation and clamping
	Rotator carRotationUsed = { 0, 0, 0 };
	Rotator rotationToApply = { 0, 0, 0 };
	Rotator currentRotation = { 0, 0, 0 };
	Vector startingVelocityTranslation = { 0, 0, 0 };
	Vector currentLocation = { 0, 0, 0 };


	bool lockRotation = true;
	bool ballBeingEdited = false;
	
	bool freezeForShot = false;



	// Clamping
	int clampVal = 0;
	bool isCeiling = false;
	float diagBound = 7965;
	float currentXBound = 4026.0f;
	float currentYBound = 5050.0f;
	float frozenZVal = 0.0f;
	bool frozeZVal = false;

	// Misc
	bool isCarRotatable = false;
	bool test = false;
	float t = 0.0f;
	
	Rotator localRotation = { 0, 0, 0 };




	void onLoad() override;
	void onUnload() override; // Uncomment and implement if you need a unload method

	// Training
	void loadHooks();
	void registerNotifiers();
	void getTrainingData(ActorWrapper cw, void* params, std::string eventName);

	ControllerManager controllerManager;


	Rotator checkForClamping(Vector loc, Rotator rot);
	Vector getClampChange(Vector loc, Rotator rot);
	std::pair<float, float> getAxisBreakDown(Rotator rot, int extra);
	Vector getStickingVelocity(Rotator rot);
	bool changeCarSpawnRotation();

	

	// Data helpers
	void CleanUp();
	int getRandomNumber(int min, int max);
	void SaveCompressedTrainingData(const std::unordered_map<std::string, CustomTrainingData>& trainingData, const std::filesystem::path& fileName);
	std::unordered_map<std::string, CustomTrainingData> LoadCompressedTrainingData(const std::filesystem::path& fileName);
	void shiftVelocitiesToPositive(std::vector<int>& vec);
	void shiftVelocitiesToNegative(std::vector<int>& vec);


	void shiftToPositive(CustomTrainingData & data);
	void shiftToNegative(CustomTrainingData& data);


	void ApplyLocalPitch(float pitchInput);




	int middleMouseIndex = 0;
	bool middleMouseReleased = true;
	bool saveCursorPos = false;
	
	bool rectangleMade = false;
	bool rectangleSaved = false;
	int backWall = 5140;

	// overview group of hooks
	void setupInputHandlingHooks();
	void setupGoalBlockerHooks();
	void setupEditorMovementHooks();
	void setupTrainingShotHooks();
	void setupTrainingEditorHooks();

	//editor movement handlers
	void handleUpdateCarData(ActorWrapper cw);
	void handleStartRound();
	void handleEditorMoveToLocation(ActorWrapper cw, void* params);
	void handleEditorSetRotation(ActorWrapper cw);
	void handleGetRotateActorCameraOffset(ActorWrapper cw);

	//goal blocker handlers
	void handleBallEditingBegin();
	void handleStopEditingGoalBlocker();
	void handleUpdateFlyPOV();

	//input handlers
	void handleTrainingEditorActorModified();
	
	//editor ui hooks
	void handleTrainingEditorEnter();
	void handleLoadRound(ActorWrapper cw, void* params, std::string eventName);
	void handleTrainingSave();
	void handleStartEditing(ActorWrapper cw);


	//shot editor hooks
	void handleUnfrozenCar(ActorWrapper car, Vector loc, Rotator rot);
	void handleFreezeCar(CarWrapper car, Vector loc, Rotator rot);
	void handleEndPlayTest();
	void handleGameEditorActorEditingEnd();
	void handleEditorModeEndState();
	void handleBallEditingEnd();
	void handleStopEditing();
	void handleStartPlayTest();
	void handleDuplicateRound(TrainingEditorWrapper cw);
	void handleDeleteRound(TrainingEditorWrapper cw);
	void handleCreateRound();
	void handleNewTrainingData(int currentShot);
	void handleExistingTrainingData(int currentShot, int totalRounds);
	void handleTrainingEditorExit();
	
	
	void replayHooks();
	void handleGetFocusCar();
	std::string focusCarID;

	bool isInReplay = false;
	

	ReplayState savedReplayState;
	
	Rotator currentRotationInTrainingEditor = { 0, 0, 0 };

	bool isInTrainingEditor();
	bool isInTrainingPack();


public:

	void RenderSettings() override; // Uncomment if you wanna render your own tab in the settings menu
	void Render(CanvasWrapper canvas);

	void onTick(std::string eventName);
	//void RenderWindow() override; // Uncomment if you want to render your own plugin window
};
