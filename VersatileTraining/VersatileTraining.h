#pragma once

#include "GuiBase.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"


#include <Windows.h>
#include <unordered_map>
#include <vector>
#include "version.h"
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);


struct CustomTrainingData {
	std::string code;
	std::string name;
	int numShots = 0;

	std::vector<int> boostAmounts;
	std::vector<int> startingVelocityMin;
	std::vector<int> startingVelocityMax;
	
};
template <typename T, typename std::enable_if<std::is_base_of<ObjectWrapper, T>::value>::type*>
void GameWrapper::HookEventWithCallerPost(std::string eventtName, std::function<void(T caller, void* params, std::string eventName)> callback)
{
	auto wrapped_callback = [callback](ActorWrapper caller, void* params, std::string eventName) {
		callback(T(caller.memory_address), params, eventName);

		};
	HookEventWithCaller<ActorWrapper>(eventtName, wrapped_callback);

}

class VersatileTraining: public BakkesMod::Plugin::BakkesModPlugin
	,public SettingsWindowBase // Uncomment if you wanna render your own tab in the settings menu
	//,public PluginWindowBase // Uncomment if you want to render your own plugin window
{

	//std::shared_ptr<bool> enabled;

	std::unordered_map<std::string, CustomTrainingData> trainingData;
	std::string editingTrainingCode;
	bool editMode = false;

	int boostAmount = -1;
	int boostMax = 100;
	int boostMin = -1;
	int startingVelocityMin = 10;
	int startingVelocityMax = 100;

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

public:
	void RenderSettings() override; // Uncomment if you wanna render your own tab in the settings menu
	//void RenderWindow() override; // Uncomment if you want to render your own plugin window
};
