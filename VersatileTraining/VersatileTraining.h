#pragma once

#include "GuiBase.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"
#include "bakkesmod/plugin/bakkesmodsdk.h"
#include "bakkesmod/wrappers/includes.h"


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


struct CustomTrainingData {
	std::string code;
	std::string name;
	int numShots = 0;
	int currentEditedShot = 0;
	std::vector<int> boostAmounts;
	std::vector<bool> freezeCar;
	std::vector<int> startingVelocity;
	bool customPack = false;

	void initCustomTrainingData(int shotAmount, std::string packName) {
		name = packName;
		numShots = shotAmount;
		currentEditedShot = 0;
		boostAmounts = std::vector<int>(shotAmount, 101);
		startingVelocity = std::vector<int>(shotAmount, 0);
		freezeCar = std::vector<bool>(shotAmount, false);

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


struct KeyState {
	int index;      // Virtual key index/code
	bool pressed;   // Is the key pressed
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
	void ApplyLocalPitch(float pitchInput);



	std::map<std::string, KeyState> keyStates;


	int middleMouseIndex = 0;
	bool saveCursorPos = false;
	std::pair<Vector,Vector> goalBlockerPos = { { 0, 0, 0 }, { 0, 0, 0 } };
public:

	void RenderSettings() override; // Uncomment if you wanna render your own tab in the settings menu
	void Render(CanvasWrapper canvas);

	void onTick(std::string eventName);
	//void RenderWindow() override; // Uncomment if you want to render your own plugin window
};
