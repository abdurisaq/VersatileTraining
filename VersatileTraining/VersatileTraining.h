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

	
	void initCustomTrainingData(int shotAmount, std::string packName) {
		name = packName;
		numShots = shotAmount;
		currentEditedShot = 0;
		boostAmounts = std::vector<int>(shotAmount, -1);
		startingVelocity = std::vector<int>(shotAmount, 0);
		freezeCar = std::vector<bool>(shotAmount, false);

	}
};
struct ButtonState {
	bool isPressed;
	std::chrono::steady_clock::time_point lastUpdateTime;
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
class VersatileTraining: public BakkesMod::Plugin::BakkesModPlugin
	,public SettingsWindowBase // Uncomment if you wanna render your own tab in the settings menu
	//,public PluginWindowBase // Uncomment if you want to render your own plugin window
{

	//std::shared_ptr<bool> enabled;

	std::unordered_map<std::string, CustomTrainingData> trainingData;
	std::string editingTrainingCode;
	bool editMode = false;

	CustomTrainingData currentTrainingData;
	CustomTrainingData *currentTrainingDataUsed;
	int currentShotIndex = 0;

	int boostAmount = -1;
	int boostMax = 100;
	int boostMin = -1;
	int maxVelocity = 2000;
	int minVelocity = -2000;
	int startingVelocity = 0;

	bool editingVariances = false;
	int tempBoostAmount = -1;
	int tempStartingVelocity = 0;

	Rotator carRotationUsed = { 0,0,0 };
	Vector startingVelocityTranslation = { 0,0,0 };
	bool appliedStartingVelocity = false;
	//Boilerplate
	void onLoad() override;
	void onUnload() override; // Uncomment and implement if you need a unload method
	void loadHooks();
	void restartTraining();
	void getTrainingData(ActorWrapper cw, void* params, std::string eventName);
	void setTrainingVariables(ActorWrapper cw, void* params, std::string eventName);

	CustomTrainingData decodeTrainingCode(std::string code);
	std::string encodeTrainingCode(CustomTrainingData data);
	TrainingEditorWrapper GetTrainingEditor();
	void onGetEditingTraining(GameEditorWrapper caller);

	//setting up button clicks to specified functions
	using ButtonCallback = std::function<void()>; 

	std::unordered_map<int, ButtonCallback> buttonCallbacks; 
	std::unordered_map<int, ButtonState> buttonStates;
	void registerButtonCallback(int buttonIndex, ButtonCallback callback);
	void initializeCallBacks();
	void IncrementTempBoost();
	void IncrementTempStartingVelocity();

	void checkForR1Press();
	void checkForButtonPress(int buttonIndex);

	bool R1AlreadyPressed = false;
	std::chrono::steady_clock::time_point lastUpdateTime;
	static BOOL CALLBACK EnumDevicesCallback(const DIDEVICEINSTANCE* instance, VOID* context);
	void enumerateControllers();


	void rollLeft();
	void rollRight();
	
	LPDIRECTINPUT8 dinput;
	LPDIRECTINPUTDEVICE8 joystick;
	std::vector<LPDIRECTINPUTDEVICE8> controllers;

	int clampVal = 0;
	Rotator checkForClamping(Vector loc, Rotator rot);
	Vector getClampChange(Vector loc, Rotator rot);
	std::pair <float, float> getAxisBreakDown(Rotator rot,int extra);
	Vector getStickingVelocity(Rotator rot);

	bool changeCarSpawnRotation();
	bool isCarRotatable = false;
	struct Input {
		int index;
		bool pressed;
		std::string name;
	};

	std::map<std::string, Input> m_inputMap;
	Rotator rotationToApply = { 0,0,0 };
	bool test = false;
	Rotator currentRotation = { 0,0,0 };
	bool lockRotation = true;
	bool freezeCar = false;
	bool freezeForShot = false;
	void CleanUp();
	int getRandomNumber(int min, int max);
	//diagBound
	float diagBound = 7965;

	//for changing roll for clamping based on ramps
	bool isCeiling = false;
	float t = 0.0f;
	float currentXBound = 4026.0f;
	float currentYBound = 5050.0f;
	float frozenZVal = 0.0f;
	bool frozeZVal = false;

	void SaveCompressedTrainingData(const std::unordered_map<std::string, CustomTrainingData>& trainingData, const std::filesystem::path& fileName);
	std::unordered_map<std::string, CustomTrainingData> LoadCompressedTrainingData(const std::filesystem::path& fileName);
	std::filesystem::path saveFilePath;
public:
	
	void RenderSettings() override; // Uncomment if you wanna render your own tab in the settings menu
	void Render(CanvasWrapper canvas);
	//void RenderWindow() override; // Uncomment if you want to render your own plugin window
};
