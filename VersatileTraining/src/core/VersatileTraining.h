#pragma once

#include "pch.h"
#include "include/GuiBase.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"
#include "bakkesmod/plugin/bakkesmodsdk.h"
#include "version.h"
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);









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

	// Rotation and clamping
	Rotator carRotationUsed = { 0, 0, 0 };
	Rotator rotationToApply = { 0, 0, 0 };
	Rotator currentRotation = { 0, 0, 0 };
	Vector startingVelocityTranslation = { 0, 0, 0 };
	Vector currentLocation = { 0, 0, 0 };
	bool lockRotation = true;
	
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
	std::filesystem::path saveFilePath;
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

	;

	// Data helpers
	void CleanUp();
	int getRandomNumber(int min, int max);
	void SaveCompressedTrainingData(const std::unordered_map<std::string, CustomTrainingData>& trainingData, const std::filesystem::path& fileName);
	std::unordered_map<std::string, CustomTrainingData> LoadCompressedTrainingData(const std::filesystem::path& fileName);
	void shiftVelocitiesToPositive(std::vector<int>& vec);
	void shiftVelocitiesToNegative(std::vector<int>& vec);

	void shiftGoalBlockerToPositive(std::vector<std::pair<Vector, Vector>>& vec);
	void shiftGoalBlockerToNegative(std::vector<std::pair<Vector, Vector>>& vec);
	void shiftToPositive(CustomTrainingData & data);
	void shiftToNegative(CustomTrainingData& data);


	void ApplyLocalPitch(float pitchInput);




	int middleMouseIndex = 0;
	bool middleMouseReleased = true;
	bool saveCursorPos = false;
	
	bool rectangleMade = false;
	bool rectangleSaved = false;
	int backWall = 5140;

	//function hook handlers
	void handleTrainingEditorActorModified();
	void handleUpdateFlyPOV();
	void handleStopEditingGoalBlocker();
	void handleBallEditingBegin();
	void handleGetRotateActorCameraOffset(ActorWrapper cw);
	void handleEditorSetRotation(ActorWrapper cw);
	void handleEditorMoveToLocation(ActorWrapper cw, void* params);
	void handleStartRound();
	void handleUnfrozenCar(ActorWrapper car, Vector loc, Rotator rot);
	void handleFreezeCar(CarWrapper car, Vector loc, Rotator rot);
	void handleUpdateCarData(ActorWrapper cw);
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
	void handleStartEditing(ActorWrapper cw);
	void handleTrainingSave();
	void handleTrainingEditorExit();
	void handleLoadRound(ActorWrapper cw, void* params, std::string eventName);
	void handleTrainingEditorEnter();

	// hooks groups together
	void setupInputHandlingHooks();
	void setupGoalBlockerHooks();
	void setupEditorMovementHooks();
	void setupTrainingShotHooks();
	void setupTrainingEditorHooks();

public:

	void RenderSettings() override; // Uncomment if you wanna render your own tab in the settings menu
	void Render(CanvasWrapper canvas);

	void onTick(std::string eventName);
	//void RenderWindow() override; // Uncomment if you want to render your own plugin window
};
