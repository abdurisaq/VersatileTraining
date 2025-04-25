#pragma once

#include "GuiBase.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"
#include "bakkesmod/plugin/bakkesmodsdk.h"
#include "bakkesmod/wrappers/includes.h"
//#include "bakkesmod/wrappers/wrapperstructs.h"
#define _AMD64_  // Assuming you are targeting 64-bit
#include <Windows.h>
#include <Xinput.h>
#pragma comment(lib, "XInput.lib")
#include <dinput.h>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")


#include <cmath>
#include <random>

#define  PI 3.14159265358979323846

#include <unordered_map>
#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>
#include "version.h"
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);

struct FVector {
	float X, Y, Z;

	FVector() : X(0), Y(0), Z(0) {}
	FVector(float x, float y, float z) : X(x), Y(y), Z(z) {}
};
struct FRotator {
	float Pitch, Yaw, Roll;

	FRotator() : Pitch(0), Yaw(0), Roll(0) {}
	FRotator(float pitch, float yaw, float roll) : Pitch(pitch), Yaw(yaw), Roll(roll) {}
};
struct pGetTrajectoryStartLocation {
	FVector ReturnValue;
};
struct pGetTrajectoryStartVelocity {
	FVector ReturnValue;
};
struct pGetTrajectoryStartRotation {
	FRotator ReturnValue;
};


struct CustomTrainingData {
	std::string code;
	std::string name;
	int numShots = 0;
	int currentEditedShot = 0;
	std::vector<int> boostAmounts;
	std::vector<bool> freezeCar;
	std::vector<int> startingVelocity;
	std::vector<std::pair<Vector, Vector>> goalBlockers;
	std::vector<std::pair<bool, bool>>goalAnchors;
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

	}
	void addShot(int boostAmount = 101, int velocity = 0, bool frozen = false , Vector firstAnchor = {0,0,0} , Vector secondAnchor = { 0,0,0 }) {
		numShots++;
		boostAmounts.push_back(boostAmount);
		startingVelocity.push_back(velocity);
		freezeCar.push_back(frozen);
		goalBlockers.push_back({ firstAnchor, secondAnchor });
		if (firstAnchor.X == 0 && firstAnchor.Z == 0 && secondAnchor.X == 0 && secondAnchor.Z == 0) {
			goalAnchors.push_back({ false, false });
			LOG("in add shot, first and second anchor are 0" );
		}
		else {
			goalAnchors.push_back({ true, true });
			LOG("in add shot, first and second anchor are not 0" );
		}
	}
};
struct ButtonState {
	bool isPressed;
	std::chrono::steady_clock::time_point lastUpdateTime;
};

struct Input {
	int index;
	bool pressed;
	std::string name;
};

struct BallState {
	Vector location;
	Rotator rotation;
	Vector velocity;
	Vector angularVelocity;

};
struct KeyState {
	int index;      // Virtual key index/code
	bool pressed;   // Is the key pressed
};

struct ShotRecording {
	int carBody = 23; //default octane
	GamepadSettings settings = GamepadSettings(0, 0.5, 1, 1);
	std::vector<ControllerInput> inputs;
	struct InitialState {
		Vector location;
		Rotator rotation;
		Vector velocity;
	};
	std::shared_ptr<InitialState> initialState;
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

	int lastRecordedFrame = -1;
	int lastPlaybackFrame = -1;
	ControllerInput lastPlaybackInput;


	BallState ballState;
	bool botSpawnedTest = false;
	bool canSpawnBot = false;
	bool primedToStartRecording = false;
	bool roundStarted = false;
	bool justStartedPlayback = false;
	int botSpawnTick = 0;
	bool startRecording = false;
	std::shared_ptr< ShotRecording> currentShotRecording;
	int frame = 0;
	int startingFrame = 0;

	bool playForNormalCar = false;

	std::unordered_map<std::string, CustomTrainingData> trainingData;
	std::string editingTrainingCode;
	bool editMode = false;

	bool editingGoalBlocker = false;
	bool goalBlockerEligbleToBeEdited = false;
	bool savedPastCamera = false;
	Vector previousLocBeforeGoalEdit = { 0, 0, 0 };
	Rotator previousRotBeforeGoalEdit = { 0, 0, 0 };

	CustomTrainingData currentTrainingData;
	CustomTrainingData* currentTrainingDataEdited;
	std::unique_ptr<CustomTrainingData> currentTrainingDataUsed;
	std::string currentPackKey;
	int currentShotIndex = 0;

	// Boost and velocity
	int boostAmount = 0;
	int boostMax = 101;
	int boostMin = 0;
	int maxVelocity = 2000;
	int minVelocity = -2000;
	int startingVelocity = 0;
	int tempBoostAmount = 0;
	int tempStartingVelocity = 2000;
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
	bool freezeCar = false;
	bool freezeForShot = false;


	//input controls
	using ButtonCallback = std::function<void()>;
	std::unordered_map<int, ButtonCallback> buttonCallbacks;
	std::unordered_map<int, ButtonState> buttonStates;
	bool R1AlreadyPressed = false;
	std::chrono::steady_clock::time_point lastUpdateTime;
	std::map<std::string, struct Input> m_inputMap;

	

	// DirectInput
	LPDIRECTINPUT8 dinput;
	LPDIRECTINPUTDEVICE8 joystick;
	std::vector<LPDIRECTINPUTDEVICE8> controllers;

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


	// controller stuff
	void registerButtonCallback(int buttonIndex, std::function<void()> callback);
	void initializeCallBacks();
	void checkForR1Press();
	void checkForButtonPress(int buttonIndex);
	void enumerateControllers();
	void IncrementTempBoost();
	void IncrementTempStartingVelocity();
	BOOL CALLBACK EnumDevicesCallback(const DIDEVICEINSTANCE* instance, VOID* context);

	// Movement helpers
	void rollLeft();
	void rollRight();

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

	void ApplyLocalPitch(float pitchInput);



	std::map<std::string, KeyState> keyStates;


	int middleMouseIndex = 0;
	bool middleMouseReleased = true;
	bool saveCursorPos = false;
	std::pair<Vector,Vector> goalBlockerPos = { { 0, 0, 0 }, { 0, 0, 0 } };
	std::pair<bool,bool> goalAnchors = { false, false };
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
	void handleFreezeCar(ActorWrapper car, Vector loc, Rotator rot);
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
	void handleExistingTrainingData(int currentShot);
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
