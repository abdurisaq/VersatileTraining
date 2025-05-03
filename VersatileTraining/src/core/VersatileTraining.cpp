#include "pch.h"
#include "src/core/VersatileTraining.h"


BAKKESMOD_PLUGIN(VersatileTraining, "write a plugin description here", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void VersatileTraining::onLoad()
{
	_globalCvarManager = cvarManager;
	 
	LOG("Plugin loaded!!");
	
	this->loadHooks();
	registerNotifiers();
	gameWrapper->RegisterDrawable([this](CanvasWrapper canvas) {
		Render(canvas);
	});
	//
	middleMouseIndex = gameWrapper->GetFNameIndexByString("MiddleMouseButton");
	gameWrapper->HookEvent("Function Engine.GameViewportClient.Tick",
		std::bind(&VersatileTraining::onTick, this, std::placeholders::_1));


	controllerManager.initializeCallBacks();

	std::filesystem::path myDataFolder = gameWrapper->GetDataFolder() / "VersatileTraining";
	saveFilePath = myDataFolder / "packs.txt";

	trainingData = LoadCompressedTrainingData(saveFilePath);
	for (auto& [key, value] : trainingData) {
		shiftToNegative(value);

		
	}

	//		
	//	}, "button pressed", PERMISSION_ALL
	//	);
	//	cvarManager->setBind(input.first, input.first + "pressed");
	//}
	
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
	setupTrainingEditorHooks();
	setupTrainingShotHooks();
	setupEditorMovementHooks();
	setupGoalBlockerHooks();
	setupInputHandlingHooks();
	replayHooks();
}








void VersatileTraining::onUnload() {
	LOG("Unloading Versatile Training");
	for (auto& [key, value] : trainingData) {
		shiftToPositive(value);
	}
	SaveCompressedTrainingData(trainingData, saveFilePath);
	CleanUp();
}

void VersatileTraining::CleanUp() {
	for (auto& controller : controllerManager.controllers) {
		if (controller) {
			controller->Unacquire(); // Unacquire before release
			controller->Release();   // Release the DirectInput device
			controller = nullptr;
		}
	}
	controllerManager.controllers.clear(); // Clear the vector
	LOG("All controllers cleaned up.");

	if (controllerManager.dinput) {
		controllerManager.dinput->Release();
		controllerManager.dinput = nullptr;
		LOG("DirectInput object released.");
	}
}



void VersatileTraining::onTick(std::string eventName) {
	
	if (editingGoalBlocker) {
		if (gameWrapper->IsKeyPressed(middleMouseIndex) && middleMouseReleased) {
			saveCursorPos = true;
			middleMouseReleased = false;
			LOG("middle mouse button pressed");
		}
		else if (!gameWrapper->IsKeyPressed(middleMouseIndex)) {
			middleMouseReleased = true;
		}
	}

	if (!(isInTrainingEditor() || isInTrainingPack())) return;

	if(!currentTrainingData.customPack) return;
	if (!currentShotState.goalAnchors.first  || !currentShotState.goalAnchors.second) return;

	ServerWrapper gameState = gameWrapper->GetCurrentGameState();

	BallWrapper ball = gameState.GetBall();

	if (!ball) {
		return;
	}

	ArrayWrapper<GoalWrapper> allGoals = gameState.GetGoals();

	if (allGoals.Count() < 2) {
		return;
	}

	Vector ballPosition = ball.GetLocation();
	Vector ballVelocity = ball.GetVelocity();

	if (ballPosition.Y > backWall && ballVelocity.Y > 0) {

		if (inRectangle(currentShotState.goalBlocker, ballPosition)) {
			GoalWrapper enemyGoal = allGoals.Get(1);

			if (!enemyGoal) {
				return;
			}

			Vector explosionPosition = ballPosition;

			ball.eventOnHitGoal(enemyGoal, explosionPosition);
			return;
		}

		ballVelocity.Y = -ballVelocity.Y;
		ball.SetVelocity(ballVelocity);
	}
}


