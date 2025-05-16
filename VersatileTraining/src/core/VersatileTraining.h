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
	,public PluginWindowBase // Uncomment if you want to render your own plugin window
{
	ShotReplicationManager shotReplicationManager;
	
	ShotState currentShotState;

	StorageManager storageManager;
	SpecialKeybinds specialKeybinds;
	std::string getKeyName(int virtualKeyCode);
	int getVirtualKeyCode(const std::string& keyName);
	void displaySpecialKeybind(const std::string& label, int& keyCode);
	
	std::filesystem::path myDataFolder;
	bool firstTime = false;
	bool canSpawnWelcomeMessage = false;

	char packSearchBuffer[128] = "";
	int packSortCriteria = 0;
	bool packSortAscending = true;
	std::string deletePackKey = "";

	std::string pastBinding;
	std::string bind_key;

	std::shared_ptr<std::unordered_map<std::string, CustomTrainingData>> trainingData;
	std::string editingTrainingCode;
	bool editMode = false;
	bool justOpenedPack = false;
	bool editingGoalBlocker = false;
	bool goalBlockerEligbleToBeEdited = false;
	bool savedPastCamera = false;
	Vector previousLocBeforeGoalEdit = { 0, 0, 0 };
	Rotator previousRotBeforeGoalEdit = { 0, 0, 0 };

	CustomTrainingData currentTrainingData;
	std::string currentPackKey;
	int currentShotIndex = 0;

	SnapShotManager snapshotManager;
	
	

	bool editingVariances = false;
	bool appliedStartingVelocity = false;
	bool appliedWallClamping = false;
	bool appliedJumpState = false;


	bool determiningCodeSync = false;
	std::string pendingCode;
	std::string pendingKey;

	// Rotation and clamping
	Rotator carRotationUsed = { 0, 0, 0 };
	Rotator rotationToApply = { 0, 0, 0 };
	Rotator currentRotation = { 0, 0, 0 };
	Vector startingVelocityTranslation = { 0, 0, 0 };
	Vector currentLocation = { 0, 0, 0 };


	bool lockRotation = true;
	bool ballBeingEdited = false;
	bool lockScene = false;
	bool unlockStartingVelocity = false;
	bool settingsOpen = false;
	bool playTestStarted = false;
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

	std::atomic<bool> serverRunning;
	std::thread serverThread;
	void runServer(std::atomic<bool>* isRunning,
		std::string playerId,
		std::shared_ptr<std::unordered_map<std::string, CustomTrainingData>> trainingDataPtr,
		std::filesystem::path dataFolder,
		std::atomic<bool>& hasAction,
		std::string& pendingAction,
		std::mutex& pendingActionMutex);
	void DownloadTrainingPackById(std::string packId);
	void checkPendingActions();
	std::atomic<bool> hasAction{ false };
	std::string pendingAction;
	std::mutex pendingActionMutex;


	LinearColor goalBlockerOutlineColor = { 0, 255, 0, 230 };  
	LinearColor goalBlockerGridColor = { 255, 255, 255, 100 }; 
	int goalBlockerGridLines = 4;
	float goalBlockerOutlineThickness = 3.0f;
	float goalBlockerGridThickness = 1.5f;

	// Function declarations
	void SetDefaultKeybinds();
	void ClearAllKeybinds();
	void readCurrentBindings();
	std::unordered_map<std::string, std::string> currentBindings;


	void onLoad() override;
	void onUnload() override; // Uncomment and implement if you need a unload method

	// Training
	void loadHooks();
	void registerNotifiers();
	void getTrainingData(ActorWrapper cw, void* params, std::string eventName);

	ControllerManager controllerManager;


	Rotator checkForClamping(Vector loc, Rotator rot);
	Vector getClampChange(Vector loc, Rotator rot);
	Rotator getCornerRotation(float cornerT, int baseYaw, int basePitch, int baseRoll);
	Rotator blendPitchRollClampSmooth(int pitch, int roll, int expectedPitch, int expectedRoll, int desiredYaw, float alpha = 1.0f);
	Rotator applyLocalPitch(Rotator rot, float blendFactor);
	std::pair<float, float> getAxisBreakDown(Rotator rot, int extra);
	Vector getStickingVelocity(Rotator rot);
	bool changeCarSpawnRotation();
	int shortestAngularDiff(int target, int current, int max_val);
	

	// Data helpers
	void CleanUp();
	int getRandomNumber(int min, int max);
	
	void shiftVelocitiesToPositive(std::vector<int>& vec);
	void shiftVelocitiesToNegative(std::vector<int>& vec);


	void shiftToPositive(CustomTrainingData & data);
	void shiftToNegative(CustomTrainingData& data);






	int middleMouseIndex = 0;
	bool middleMouseReleased = true;
	bool saveCursorPos = false;
	
	bool rectangleMade = false;
	bool rectangleSaved = false;
	float backWall = 5140.0f;

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


	void RenderEnhancedGoalBlocker(
		CanvasWrapper& canvas,
		CameraWrapper& camera,
		RT::Frustum& frustum,
		BallWrapper& ball,
		const Vector& topLeft,
		const Vector& topRight,
		const Vector& bottomLeft,
		const Vector& bottomRight);

	void DrawGoalBlockerGrid(
        CanvasWrapper& canvas,
        CameraWrapper& camera,
        RT::Frustum& frustum,
        BallWrapper& ball,
        CarWrapper& car, // Added car parameter
        const Vector& topLeft,
        const Vector& topRight,
        const Vector& bottomLeft,
        const Vector& bottomRight);

	void RenderEnhancedGoalBlocker_WithCarClip(
		CanvasWrapper& canvas,
		CameraWrapper& camera,
		RT::Frustum& frustum,
		BallWrapper& ball,
		CarWrapper& car,
		const Vector& topLeft,
		const Vector& topRight,
		const Vector& bottomLeft,
		const Vector& bottomRight);

	void DrawGoalBlockerGrid_WithCarClip(
		CanvasWrapper& canvas,
		CameraWrapper& camera,
		RT::Frustum& frustum,
		BallWrapper& ball,
		CarWrapper& car,
		const Vector& topLeft,
		const Vector& topRight,
		const Vector& bottomLeft,
		const Vector& bottomRight);

	Vector LerpVector(const Vector& a, const Vector& b, float t);
	LinearColor LerpColor(const LinearColor& a, const LinearColor& b, float t);


	//gui

	std::string lastCopiedPackCode;
	float packCodeCopyFlashTimer = 0.0f;

	char allSnapshotsSearchBuffer[128];
    int allSnapshotsFilterType;
    int allSnapshotsSourceFilter;
    bool allSnapshotsSortAscending;
	bool requestOpenAddSnapshotsPopup = false;
    bool requestOpenRenameGroupPopup = false;
    
    std::vector<SnapshotGroup> snapshotGroups;
    
    char newGroupNameInput[128];        
     char renameGroupNameInput[128];      
     std::string groupToRenameUid;        
     std::string groupToAddToUid;         
     std::vector<bool> snapshotSelectionForAdding; 
     char groupSearchBuffer[128];        
     char snapshotSearchInAddPopupBuffer[128]; 

    void UpdateGroupIndicesAfterSnapshotDeletion(size_t deletedOriginalIndex);


    //Rendering Functions 

    void RenderSnapshotDetailsInGroup(SnapshotGroup& group, size_t snapshotManagerIndex, size_t displayIndexInGroupList);
    void RenderCreateGroupPopup();
    void RenderRenameGroupPopup();
    void RenderAddSnapshotsToGroupPopup();
public:

	void RenderAllSnapshotsTab();
	void RenderGroupedSnapshotsTab();
	void LoadSnapshotGroups();
	void SaveSnapshotGroups();


	void RenderSettings() override; // Uncomment if you wanna render your own tab in the settings menu
	void Render(CanvasWrapper canvas);
	void RenderVelocityOfCar(CanvasWrapper canvas);
	void onTick(std::string eventName);
	void RenderWindow() override; // Uncomment if you want to render your own plugin window
};
