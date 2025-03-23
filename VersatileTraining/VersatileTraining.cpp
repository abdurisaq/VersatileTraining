#include "pch.h"
#include "VersatileTraining.h"


BAKKESMOD_PLUGIN(VersatileTraining, "write a plugin description here", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void VersatileTraining::onLoad()
{
	_globalCvarManager = cvarManager;

	
	LOG("Plugin loaded!!");
	
	this->loadHooks();
	
	/*gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.GameEvent_TrainingEditor_TA.LoadRound", [this](ActorWrapper cw, void* params, std::string eventName) {
		VersatileTraining::getTrainingData(cw, params, eventName);
		});*/

	/*gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.GameEvent_TrainingEditor_TA.LoadRound", [this](ActorWrapper cw, void* params, std::string eventName) {
		auto tw = ((TrainingEditorWrapper)cw.memory_address);
		GameEditorSaveDataWrapper data = tw.GetTrainingData();
		TrainingEditorSaveDataWrapper td = data.GetTrainingData();
		LOG("Training data found: {}", td.GetCode().ToString());
		});*/
	// !! Enable debug logging by setting DEBUG_LOG = true in logging.h !!
	//DEBUGLOG("VersatileTraining debug mode enabled");

	// LOG and DEBUGLOG use fmt format strings https://fmt.dev/latest/index.html
	//DEBUGLOG("1 = {}, 2 = {}, pi = {}, false != {}", "one", 2, 3.14, true);

	//cvarManager->registerNotifier("my_aweseome_notifier", [&](std::vector<std::string> args) {
	//	LOG("Hello notifier!");
	//}, "", 0);

	//auto cvar = cvarManager->registerCvar("template_cvar", "hello-cvar", "just a example of a cvar");
	//auto cvar2 = cvarManager->registerCvar("template_cvar2", "0", "just a example of a cvar with more settings", true, true, -10, true, 10 );

	//cvar.addOnValueChanged([this](std::string cvarName, CVarWrapper newCvar) {
	//	LOG("the cvar with name: {} changed", cvarName);
	//	LOG("the new value is: {}", newCvar.getStringValue());
	//});

	//cvar2.addOnValueChanged(std::bind(&VersatileTraining::YourPluginMethod, this, _1, _2));

	// enabled decleared in the header
	//enabled = std::make_shared<bool>(false);
	//cvarManager->registerCvar("TEMPLATE_Enabled", "0", "Enable the TEMPLATE plugin", true, true, 0, true, 1).bindTo(enabled);

	//cvarManager->registerNotifier("NOTIFIER", [this](std::vector<std::string> params){FUNCTION();}, "DESCRIPTION", PERMISSION_ALL);
	//cvarManager->registerCvar("CVAR", "DEFAULTVALUE", "DESCRIPTION", true, true, MINVAL, true, MAXVAL);//.bindTo(CVARVARIABLE);
	//gameWrapper->HookEvent("FUNCTIONNAME", std::bind(&TEMPLATE::FUNCTION, this));
	//gameWrapper->HookEventWithCallerPost<ActorWrapper>("FUNCTIONNAME", std::bind(&VersatileTraining::FUNCTION, this, _1, _2, _3));
	//gameWrapper->RegisterDrawable(bind(&TEMPLATE::Render, this, std::placeholders::_1));


	//gameWrapper->HookEvent("Function TAGame.Ball_TA.Explode", [this](std::string eventName) {
	//	LOG("Your hook got called and the ball went POOF");
	//});
	// You could also use std::bind here
	//gameWrapper->HookEvent("Function TAGame.Ball_TA.Explode", std::bind(&VersatileTraining::YourPluginMethod, this);
}

void VersatileTraining::loadHooks() {

	/*gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.GameEvent_TrainingEditor_TA.LoadRound", [this](ActorWrapper cw, void* params, std::string eventName) {
		VersatileTraining::getTrainingData(cw, params, eventName);
		});*/

	gameWrapper->HookEventWithCallerPost<GameEditorWrapper>("Function GameEvent_TrainingEditor_TA.ShotSelection.StartEditing", [this](GameEditorWrapper cw, void* params, std::string eventName) {
		LOG("new training pack opened and editing started--------------");
		});

	

	//gameWrapper->HookEventWithCaller<TrainingEditorWrapper>(
	//	"Function TAGame.TrainingEditor_TA.SetTracedCrosshairActor",
	//	[this](TrainingEditorWrapper te, void* params, std::string eventName) {
	//		ActorWrapper* actor = (ActorWrapper*)params;
	//		unsigned char physicsType = actor->GetPhysics();
	//		LOG("Physics type: {}", physicsType);
	//		/*if (actor && actor->IsA("Car_TA")) {
	//			LOG("Car is currently focused in the training editor.-----------------------------");
	//		}
	//		else if (actor && actor->IsA("Ball_TA")) {
	//			LOG("Ball is currently focused in the training editor.--------------------");
	//		}*/
	//	}
	//);

	

	/*gamewrapper->hookeventwithcallerpost<gameeditorwrapper>("function tagame.gameevent_trainingeditor_ta.shotselection.startediting", [this](gameeditorwrapper cw, void* params, std::string eventname) {
		log("new training pack opened and editing started--------------");
		serverwrapper sw = gamewrapper->getcurrentgamestate();
		if (!sw) return;
		gamesettingplaylistwrapper playlist = sw.getplaylist();
		if (!playlist) return;
		int playlistid = playlist.getplaylistid();
		if (playlistid == 20) {
			log("training editor is open----------------");

		}
		});*/

	/*gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.GameEvent_TrainingEditor_TA.OnInit", [this](ActorWrapper cw, void* params, std::string eventName) {
			
		LOG("New Training pack opened--------------");
		ServerWrapper sw = gameWrapper->GetCurrentGameState();
		if (!sw) return;
		GameSettingPlaylistWrapper playlist = sw.GetPlaylist();
		if (!playlist) return;
		int playlistID = playlist.GetPlaylistId();
		if (playlistID == 20) {
			LOG("Training editor is open----------------");

		}
		});*/

		
}

TrainingEditorWrapper VersatileTraining::GetTrainingEditor() {
	auto serv = gameWrapper->GetGameEventAsServer();
	if (!serv) return { 0 };
	return TrainingEditorWrapper(serv.memory_address);
}

void VersatileTraining::getTrainingData(ActorWrapper cw, void* params, std::string eventName) {

	/*TrainingEditorWrapper tew = GetTrainingEditor();
	if (!tew) return;

	auto current = tew.GetActiveRoundNumber();*/

	auto tw = ((TrainingEditorWrapper)cw.memory_address);
	GameEditorSaveDataWrapper data = tw.GetTrainingData();
	TrainingEditorSaveDataWrapper td = data.GetTrainingData();
	LOG("Training data found1: {}", td.GetCode().ToString());
	int totalRounds = tw.GetTotalRounds();
	LOG("Training num rounds: {}", totalRounds);

	int currentShot = tw.GetActiveRoundNumber();
	LOG("Training current shot : {}", currentShot);

	std::string code = td.GetCode().ToString();

	auto foundCode =  trainingData.find(code);
	if (foundCode != trainingData.end()) {
		LOG("Training data found: {}", code);
		foundCode->second.name = td.GetTM_Name().ToString();
		foundCode->second.numShots = totalRounds;
		LOG("loaded training name: {}", foundCode->second.name);
		cvarManager->executeCommand("sv_training_enabled 1");
		cvarManager->executeCommand("sv_training_limitboost " + std::to_string(foundCode->second.boostAmounts[0]));
		//sv_training_player_velocity (1700, 1900);
		cvarManager->executeCommand("sv_training_player_velocity ("+ std::to_string(foundCode->second.startingVelocityMin[0]) + "," + std::to_string(foundCode->second.startingVelocityMax[0]) + ")");

	}
	else {
		LOG("Training data not found: {}", code);
		cvarManager->executeCommand("sv_training_enabled 0");
		cvarManager->executeCommand("sv_training_limitboost -1");

	}
	/*if (code == "0624-600D-000E-F2A7") {
		cvarManager->executeCommand("sv_training_enabled 1");
		cvarManager->executeCommand("sv_training_limitboost 100");
	}
	else {
		cvarManager->executeCommand("sv_training_enabled 0");
		cvarManager->executeCommand("sv_training_limitboost -1");

	}*/

	//executecommand ("sv_training_enabled 1); to turn on custom training variance, and if not turn it off.
	
}

void VersatileTraining::setTrainingVariables(ActorWrapper cw, void* params, std::string eventName) {

}

void VersatileTraining::restartTraining() {
	LOG("Restarting training");
	ServerWrapper server = gameWrapper->GetGameEventAsServer();

	TrainingEditorWrapper tew(server.memory_address);

	

}

void VersatileTraining::onUnload() {
	LOG("Unloading Versatile Training");
}

//std::vector<std::pair<int, int>> runLengthEncode(std::vector<int> arr) {
//
//	std::vector<std::pair<int, int>> result;
//	int count = 1;
//	int pastInt = arr[0];
//	for (int i = 1; i < arr.size(); i++) {
//		if (arr[i] == pastInt) {
//			count++;
//		}
//		else {
//			result.push_back(std::make_pair(pastInt, count));
//			count = 1;
//			pastInt = arr[i];
//		}
//	}
//
//}

//
//std::string VersatileTraining::encodeTrainingCode(CustomTrainingData data) {
//
//	std::string start = "";
//	std::string header = data.code + "-"+ std::to_string(data.numShots);
//	std::vector<int>deltaChangeInBoost;
//	std::vector<int>deltaChangeMinVelocity;
//	std::vector<int>deltaChangeMaxVelocity;
//	int pastMinVelocity = 0;
//	int pastMaxVelocity = 0;
//	int pastBoost = 0;
//	for (int boostAmount : data.boostAmounts) {
//		deltaChangeInBoost.push_back(boostAmount - pastBoost);
//	}
//	for (int minVelocity : data.startingVelocityMin) {
//		deltaChangeMinVelocity.push_back(minVelocity - pastMinVelocity);
//	}
//	for (int maxVelocity : data.startingVelocityMax) {
//		deltaChangeMaxVelocity.push_back(maxVelocity - pastMaxVelocity);
//	}
//
//
//	return "";
//}