#include "pch.h"
#include "VersatileTraining.h"


BAKKESMOD_PLUGIN(VersatileTraining, "write a plugin description here", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void VersatileTraining::onLoad()
{
	_globalCvarManager = cvarManager;
	LPDIRECTINPUT8 directInput = nullptr;
	LPDIRECTINPUTDEVICE8 controller = nullptr;
	
	LOG("Plugin loaded!!");
	
	this->loadHooks();
	
	gameWrapper->RegisterDrawable([this](CanvasWrapper canvas) {
		Render(canvas);
	});
	
	cvarManager->registerNotifier(
		"findController",
		[this](std::vector<std::string> args) {
			HWND hwnd = FindWindowA("LaunchUnrealUWindowsClient", "Rocket League (64-bit, DX11, Cooked)");
			if (!hwnd) {
				LOG("Failed to get Rocket League window handle.");
				return;
			}
			else {
				LOG("Got Rocket League window handle.");
			}

			enumerateControllers();
		}, "check to see if there's a connected controller", PERMISSION_ALL
	);
	cvarManager->registerNotifier("unlockCar", [this](std::vector<std::string> args){
		
		if (lockRotation) {
			LOG("car unlocked");
		}
		else {
			LOG("car locked");
		}
		lockRotation = !lockRotation;

		
		}, "unlock car", PERMISSION_ALL);

	cvarManager->setBind("X","unlockCar");
	cvarManager->registerNotifier("freezeCar", [this](std::vector<std::string> args) {

		if (freezeCar) {
			LOG("car unfrozen");
		}
		else {
			LOG("car frozen");
		}
		freezeCar = !freezeCar;
		if (currentTrainingDataUsed != nullptr) {
			//currentTrainingDataUsed->freezeCar[currentTrainingDataUsed->currentEditedShot] = freezeCar;
			currentTrainingData.freezeCar[currentTrainingData.currentEditedShot] = freezeCar;
		}

		
		}, "freeze car", PERMISSION_ALL);

	cvarManager->setBind("F", "freezeCar");

	cvarManager->registerNotifier("printDataMap", [this](std::vector<std::string> args) {

		for (auto [key, value] : trainingData) {
			LOG("Name: {}", value.name);
			LOG("Code: {}", key);
			LOG("Num Shots: {}", value.numShots);
			for (int i = 0; i < value.numShots; i++) {
				LOG("Shot {}: Boost Amount: {}, Starting Velocity: {}, Freeze Car: {}", i, value.boostAmounts[i], value.startingVelocity[i], static_cast<int>(value.freezeCar[i]));
			}//i, value.boostAmounts[i], value.startingVelocity[i], value.freezeCar[i]
			LOG("------------------------------");
		}


		}, "print local data map", PERMISSION_ALL);

	cvarManager->setBind("L", "printDataMap");

	cvarManager->registerNotifier("printCurrentPack", [this](std::vector<std::string> args) {

		//currentTrainingData
		LOG("Name: {}", currentTrainingData.name);
		LOG("Code: {}", currentPackKey);
		LOG("Num Shots: {}", currentTrainingData.numShots);
		for (int i = 0; i < currentTrainingData.numShots; i++) {
			LOG("Shot {}: Boost Amount: {}, Starting Velocity: {}, Freeze Car: {}", i, currentTrainingData.boostAmounts[i], currentTrainingData.startingVelocity[i], static_cast<int>(currentTrainingData.freezeCar[i]));
		}//i, value.boostAmounts[i], value.startingVelocity[i], value.freezeCar[i]

		}, "print local data map", PERMISSION_ALL);

	cvarManager->setBind("P", "printCurrentPack");


	initializeCallBacks();

	std::filesystem::path myDataFolder = gameWrapper->GetDataFolder() / "VersatileTraining";
	saveFilePath = myDataFolder / "packs.txt";

	trainingData = LoadCompressedTrainingData(saveFilePath);
	for (auto& [key, value] : trainingData) {
		shiftVelocitiesToNegative(value.startingVelocity);
	}
	//m_inputMap["Up"] = { 0, false, "Up" };
	//m_inputMap["Down"] = { 0, false, "Down" };
	//m_inputMap["Left"] = { 0, false, "Left" };
	//m_inputMap["Right"] = { 0, false, "Right" };

	//// Register the arrow keys to update the rotation
	//for (const auto& input : m_inputMap) {
	//	cvarManager->registerNotifier(input.first+"pressed", [this](std::vector<std::string> args) {
	//		if (editingVariances) {
	//			if (args[0] == "Uppressed") {
	//				rotationToApply.Pitch += 500;
	//				LOG("Up arrow pressed");
	//			}
	//			if (args[0] == "Downpressed") {
	//				rotationToApply.Pitch -= 500;
	//				LOG("Down arrow pressed");
	//			}
	//			if (args[0] == "Leftpressed") {
	//				rotationToApply.Roll -= 500;
	//				LOG("Left arrow pressed");
	//			}
	//			if (args[0] == "Rightpressed") {
	//				rotationToApply.Roll += 500;
	//				LOG("Right arrow pressed");
	//			}
	//		}
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

	//Function TAGame.TrainingEditorMetrics_TA.TrainingEditorEnter
	gameWrapper->HookEventWithCallerPost<ActorWrapper>("Function TAGame.TrainingEditorMetrics_TA.TrainingEditorEnter", [this](ActorWrapper cw, void* params, std::string eventName) {
		LOG("Training editor enter");
		currentTrainingData.currentEditedShot = -1;
		
	});
	gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.GameEvent_TrainingEditor_TA.LoadRound", [this](ActorWrapper cw, void* params, std::string eventName) {
		
		VersatileTraining::getTrainingData(cw, params, eventName);
		freezeForShot = freezeCar;
		frozeZVal = !freezeCar;
		lockRotation = true;
		appliedStartingVelocity = false;
		editingVariances = false;
		appliedWallClamping = false;
		});

	//TAGame.TrainingEditorMetrics_TA.TrainingEditorExit save when this is called
	//
	//TAGame.GameEvent_TrainingEditor_TA.GetTrainingMetrics
	gameWrapper->HookEventWithCallerPost<ActorWrapper>("Function TAGame.TrainingEditorMetrics_TA.TrainingEditorExit", [this](ActorWrapper cw, void* params, std::string eventName) {
		currentTrainingData.currentEditedShot = -1;
		});

	gameWrapper->HookEventWithCallerPost<ActorWrapper>("Function TAGame.GameEvent_TrainingEditor_TA.Save", [this](ActorWrapper cw, void* params, std::string eventName) {
		//if saved is called, then write what i've been doing to memory, if not, free and discard.
		if (currentTrainingData.currentEditedShot != -1) {
			//*currentTrainingDataEdited = *currentTrainingDataUsed; // Proper deep copy //crashes it. need to make a copy constructor, and make the = operator work
			//trainingData[currentPackKey] = *currentTrainingDataUsed;
			currentTrainingData.boostAmounts[currentTrainingData.currentEditedShot] = tempBoostAmount;
			currentTrainingData.startingVelocity[currentTrainingData.currentEditedShot] = tempStartingVelocity;
			currentTrainingData.freezeCar[currentTrainingData.currentEditedShot] = freezeCar;
			trainingData[currentPackKey] = currentTrainingData;


			//currentTrainingData
			for (auto& [key, value] : trainingData) {
				shiftVelocitiesToPositive(value.startingVelocity);
			}
			SaveCompressedTrainingData(trainingData, saveFilePath);//crashe for some reason
			trainingData = LoadCompressedTrainingData(saveFilePath);
			for (auto& [key, value] : trainingData) {
				shiftVelocitiesToNegative(value.startingVelocity);
			}
		}
		/*currentPackKey.clear();
		currentTrainingDataUsed.reset();
		currentTrainingDataEdited = nullptr;*/
	});

	gameWrapper->HookEventWithCallerPost<ActorWrapper>("Function GameEvent_TrainingEditor_TA.ShotSelection.StartEditing", [this](ActorWrapper cw, void* params, std::string eventName) {
		LOG("new training pack opened and editing started--------------");
		isCarRotatable = true;
		if (!cw) {
			LOG("Caller is invalid");
			return;
		}

		TrainingEditorWrapper tw(cw.memory_address);
		if (tw.IsNull()) {
			LOG("Failed to get TrainingEditorWrapper");
			return;
		}

		GameEditorSaveDataWrapper data = tw.GetTrainingData();
		TrainingEditorSaveDataWrapper td = data.GetTrainingData();

		std::string name = td.GetTM_Name().ToString();
		int currentShot = tw.GetActiveRoundNumber();
		int totalRounds = tw.GetTotalRounds();
		
		LOG("Training pack name : {}", name);
		
		LOG("Training current shot : {}", currentShot);
		LOG("Training num rounds: {}", totalRounds);
		
		//iterate over trainingData and look for name
		if (currentTrainingData.currentEditedShot != -1) {
			LOG("already loaded, skipping searching training data");
			LOG("currentShot: {}", currentTrainingData.currentEditedShot);
			LOG("setting boost atmount to {}", currentTrainingData.boostAmounts[currentShot]);
			tempBoostAmount = currentTrainingData.boostAmounts[currentShot];
			LOG("setting starting velocity to {}", currentTrainingData.startingVelocity[currentShot]);
			tempStartingVelocity = currentTrainingData.startingVelocity[currentShot];
			currentTrainingData.currentEditedShot = currentShot;
			LOG("setting freeze car to {}", currentTrainingData.freezeCar[currentShot] ? "false" : "true");
			freezeCar = currentTrainingData.freezeCar[currentShot];
			//currentTrainingDataUsed->currentEditedShot = currentShot;
			return;
		}
		bool found = false;
		for (auto& [key, value] : trainingData) {
			if (value.name == name) {
				currentTrainingData = value;
				LOG("Training pack found in trainingData");
				found = true;
				currentPackKey = key;
				//currentTrainingDataEdited = &trainingData[key]; 
				//currentTrainingDataUsed = std::make_unique<CustomTrainingData>(value);
				break;
			}
		}
		if (!found) {
			LOG("Training pack not found in trainingData");
			currentTrainingData.initCustomTrainingData(totalRounds, name);
			auto [it, inserted] = trainingData.insert_or_assign(name, currentTrainingData);
			//currentTrainingDataEdited = &it->second;
			currentPackKey = name;
			//currentTrainingDataUsed = std::make_unique<CustomTrainingData>(*currentTrainingDataEdited);
			
			//return;
		}

		LOG("setting boost atmount to {}", currentTrainingData.boostAmounts[currentShot]);
		tempBoostAmount = currentTrainingData.boostAmounts[currentShot];
		LOG("setting starting velocity to {}", currentTrainingData.startingVelocity[currentShot]);
		tempStartingVelocity= currentTrainingData.startingVelocity[currentShot];
		currentTrainingData.currentEditedShot = currentShot;
		LOG("setting freeze car to {}", currentTrainingData.freezeCar[currentShot] ? "false" : "true");
		freezeCar = currentTrainingData.freezeCar[currentShot];
		//currentTrainingDataUsed->currentEditedShot = currentShot;
		
		
		
		});

	//TAGame.GFxData_TrainingModeEditor_TA.CreateRound
	gameWrapper->HookEventWithCallerPost<TrainingEditorWrapper>("Function TAGame.GFxData_TrainingModeEditor_TA.CreateRound", [this](TrainingEditorWrapper cw, void* params, std::string eventName) {
		currentTrainingData.numShots++;
		currentTrainingData.boostAmounts.push_back(101);
		currentTrainingData.startingVelocity.push_back(2000);
		currentTrainingData.freezeCar.push_back(false);
		
		
		});
	//TAGame.GameEvent_TrainingEditor_TA.DeleteRound
	gameWrapper->HookEventWithCallerPost<TrainingEditorWrapper>("Function TAGame.GameEvent_TrainingEditor_TA.DeleteRound", [this](TrainingEditorWrapper cw, void* params, std::string eventName) {
		int currentShot = cw.GetActiveRoundNumber();

		LOG("removing shot: {}", currentShot);
		currentTrainingData.numShots--;
		currentTrainingData.boostAmounts.erase(currentTrainingData.boostAmounts.begin() + currentShot);
		currentTrainingData.startingVelocity.erase(currentTrainingData.startingVelocity.begin() + currentShot);
		currentTrainingData.freezeCar.erase(currentTrainingData.freezeCar.begin() + currentShot);


		});
	gameWrapper->HookEventWithCallerPost<TrainingEditorWrapper>("Function GameEvent_TrainingEditor_TA.ShotSelection.DuplicateRound", [this](TrainingEditorWrapper cw, void* params, std::string eventName) {
		//duplicate shot starting velocity and boost amount, and frozen status
		int currentShot = cw.GetActiveRoundNumber();
		int numRounds = cw.GetTotalRounds();
		LOG("duplicating shot {}", currentShot);
		LOG("num rounds {}", numRounds);
		LOG("copying a shot that is frozen ? {}", currentTrainingData.freezeCar[currentShot] ? "true" : "false");
		int boostToCopy = currentTrainingData.boostAmounts[currentShot];
		int startingVelocityToCopy = currentTrainingData.startingVelocity[currentShot];
		bool freezeToCopy = currentTrainingData.freezeCar[currentShot];
		currentTrainingData.boostAmounts.push_back(boostToCopy);
		currentTrainingData.startingVelocity.push_back(startingVelocityToCopy);
		currentTrainingData.freezeCar.push_back(freezeToCopy);
		currentTrainingData.numShots++;
		



		});

	gameWrapper->HookEventWithCallerPost<TrainingEditorWrapper>("Function TAGame.GameEvent_TrainingEditor_TA.StartPlayTest", [this](TrainingEditorWrapper cw, void* params, std::string eventName) {
		if (!currentTrainingData.customPack) return;
		LOG("setting boost amount in start play test to {}", tempBoostAmount);
		currentTrainingData.boostAmounts[currentTrainingData.currentEditedShot] = tempBoostAmount;
		LOG("setting starting velocity in start play test to {}", tempStartingVelocity);
		currentTrainingData.startingVelocity[currentTrainingData.currentEditedShot] = tempStartingVelocity;
		LOG("setting freeze car in start play test to {}", freezeCar ? "false" : "true");
		currentTrainingData.freezeCar[currentTrainingData.currentEditedShot] = freezeCar;
		trainingData[currentTrainingData.name] = currentTrainingData;

		});

	gameWrapper->HookEventWithCallerPost<TrainingEditorWrapper>("Function GameEvent_TrainingEditor_TA.EditorMode.StopEditing", [this](TrainingEditorWrapper cw, void* params, std::string eventName) {
		if (!currentTrainingData.customPack) return;
		LOG("stopped editing");
		isCarRotatable = false;
		lockRotation = true;
		editingVariances = false;
		if (currentTrainingData.currentEditedShot == -1) {
			LOG("currentTrainingDataUsed is null");
			return;
		}
		LOG("saving to shot in training data {}", currentTrainingData.currentEditedShot);
		LOG("temp boost amount: {}", tempBoostAmount);
		
		currentTrainingData.boostAmounts[currentTrainingData.currentEditedShot] = tempBoostAmount;
		currentTrainingData.startingVelocity[currentTrainingData.currentEditedShot] = tempStartingVelocity;
		currentTrainingData.freezeCar[currentTrainingData.currentEditedShot] = freezeCar;
		trainingData[currentTrainingData.name] = currentTrainingData;
		
		LOG("adding shot training data: {}, boost amount: {}, starting velocity: {}", currentTrainingData.currentEditedShot, currentTrainingData.boostAmounts[currentTrainingData.currentEditedShot], currentTrainingData.startingVelocity[currentTrainingData.currentEditedShot]);


		});

	gameWrapper->HookEventWithCallerPost<TrainingEditorWrapper>("Function TAGame.Ball_GameEditor_TA.EditingEnd", [this](TrainingEditorWrapper cw, void* params, std::string eventName) {
		LOG("car is being edited");
		editingVariances = true;

		});
	//GameEvent_GameEditor_TA.EditorMode.EndState
	gameWrapper->HookEventWithCallerPost<TrainingEditorWrapper>("Function GameEvent_GameEditor_TA.EditorMode.EndState", [this](TrainingEditorWrapper cw, void* params, std::string eventName) {
		editingVariances = false;

		});
	gameWrapper->HookEventWithCallerPost<TrainingEditorWrapper>("Function TAGame.GameEditor_Actor_TA.EditingEnd", [this](TrainingEditorWrapper cw, void* params, std::string eventName) {
		LOG("ball is being edited");
		editingVariances = false;
		lockRotation = true;
		});
	//TAGame.GameEvent_TrainingEditor_TA.EventPlaytestStarted
	/*gameWrapper->HookEvent("Function TAGame.GameEvent_TrainingEditor_TA.EventPlaytestStarted", [this](std::string eventName) {
		isCarRotatable = false;

		});*/
	gameWrapper->HookEvent("Function TAGame.GameEvent_TrainingEditor_TA.EndPlayTest", [this](std::string eventName) {
		isCarRotatable = true;

		});
	//TAGame.GFxHUD_TA.UpdateCarData
	//TAGame.GameEditor_Actor_TA.EditorMoveToLocation call this to go beyond the bounds maybe, see if i can go beyond the border, if past location is the same as current location, and if pitch and roll aren't 0, let me go through the bounds a bit
	gameWrapper->HookEventWithCallerPost<ActorWrapper >("Function TAGame.GFxHUD_TA.UpdateCarData", [this](ActorWrapper  cw, void* params, std::string eventName) {
		if (!currentTrainingData.customPack) return;
		if (editingVariances && !lockRotation) {
			if (!cw || cw.IsNull()) {
				LOG("Server not found");
				return;
			}


		}
		cw.SetbCollideWorld(0);
		if (gameWrapper->IsInCustomTraining()) {
			ServerWrapper server = gameWrapper->GetCurrentGameState();
			if (!server) { return; }
			ActorWrapper car = server.GetGameCar();
			if (!car) {
				//LOG("Car not found");
				return;
			}
			Rotator rot = car.GetRotation();
			Vector loc = car.GetLocation();
			if (!appliedWallClamping) {
				LOG("applying clamping to wall");
				Vector loc2 = getClampChange(loc, rot);
				//if (loc2.X != 0 || loc2.Y != 0 || loc2.Z != 0) {

					car.SetLocation(loc2);
				//}
				appliedWallClamping = true;
			}

			if (freezeForShot) {
				if (gameWrapper->IsInCustomTraining()) {
					//LOG("In custom training");


					/*Rotator rot = car.GetRotation();
					Vector loc = car.GetLocation();*/
					if (rot.Pitch != carRotationUsed.Pitch || rot.Yaw != carRotationUsed.Yaw) {

						LOG("Current Rotation - Pitch: {}, Yaw: {}, Roll: {}", rot.Pitch, rot.Yaw, rot.Roll);
						LOG("Current location - X: {}, Y: {}, Z: {}", loc.X, loc.Y, loc.Z);
						//car.SetLocation({ loc.X,5090,loc.Z });
						if (!frozeZVal) {
							frozenZVal = loc.Z;
							frozeZVal = true;
						}
						Vector loc2 = getClampChange(loc, rot);
						if (loc2.X != 0 || loc2.Y != 0 || loc2.Z != 0) {

							car.SetLocation(loc2);
						}
						float pitchRad = (rot.Pitch / 16201.0f) * (PI / 2);
						float yawRad = (rot.Yaw / 32768.0f) * PI;
						float z = sinf(pitchRad);
						float y = cosf(pitchRad) * sinf(yawRad);
						float x = cosf(pitchRad) * cosf(yawRad);
						Vector unitVector = { x,y,z };
						int velocity = currentTrainingData.startingVelocity[currentTrainingData.currentEditedShot];//getRandomNumber(tempStartingVelocityMin, tempStartingVelocityMax);//changed 
						startingVelocityTranslation = unitVector * velocity;
					}
					car.SetAngularVelocity(Vector{ 0, 0, 0 }, false);
					Vector vel = car.GetVelocity();
					car.SetVelocity({ vel.X,vel.Y,5 });
					LOG("keeping car frozen");
					LOG("starting velocity - X: {}, Y: {}, Z: {}", startingVelocityTranslation.X, startingVelocityTranslation.Y, startingVelocityTranslation.Z);
					//first time it isn't frozen, i need to apply the new velocity caluclated by x y z components of magnitude velocity chosen at random between min and max

				}
			}
			else if (!freezeForShot && gameWrapper->IsInCustomTraining() && !appliedStartingVelocity) {
				/*ServerWrapper server = gameWrapper->GetCurrentGameState();
				if (!server) return;
				ActorWrapper car = server.GetGameCar();
				if (!car) return;

				Vector loc = car.GetLocation();
				Rotator rot = car.GetRotation();*/

				/*Vector stickingVelocity = getStickingVelocity(rot);
				car.SetVelocity(startingVelocityTranslation + stickingVelocity);*/

				if (!frozeZVal) {
					frozenZVal = loc.Z;
					frozeZVal = true;
				}
				Vector loc2 = getClampChange(loc, rot);
				/*LOG("clamp changes starting location X: {}, Y: {}, Z: {}", loc.X, loc.Y, loc.Z);
				if (loc2.X != 0 || loc2.Y != 0 || loc2.Z != 0) {
					car.SetLocation(loc2);
				}*/
				appliedStartingVelocity = true;
			}
		}
		});
	
	gameWrapper->HookEvent("Function GameEvent_Soccar_TA.Active.StartRound", [this](std::string eventName) {
		if (gameWrapper->IsInCustomTraining()) {
			if (!currentTrainingData.customPack) return;
			freezeForShot = false;
			frozeZVal = false;
			appliedStartingVelocity = false; // mark as not yet applied

			// Get car and calculate velocity direction vector
			ServerWrapper server = gameWrapper->GetCurrentGameState();
			if (!server) return;
			ActorWrapper car = server.GetGameCar();
			if (!car) return;

			Rotator rot = car.GetRotation();

			float pitchRad = (rot.Pitch / 16201.0f) * (PI / 2);
			float yawRad = (rot.Yaw / 32768.0f) * PI;
			float z = sinf(pitchRad);
			float y = cosf(pitchRad) * sinf(yawRad);
			float x = cosf(pitchRad) * cosf(yawRad);
			Vector unitVector = { x, y, z };

			int velocity = currentTrainingData.startingVelocity[currentTrainingData.currentEditedShot];
			if (velocity == 0) return;
			startingVelocityTranslation = unitVector * velocity;
			Vector stickingVelocity = getStickingVelocity(rot);
			car.SetVelocity(startingVelocityTranslation + stickingVelocity);

			LOG("Calculated new velocity in StartRound");
		}
	});
	///TAGame.GameEditor_Actor_TA.CanEdit
//Engine.Actor.HitWall

	struct pExecEditorMoveToLocaction
	{
		struct Vector NewLocation;
		uint32_t ReturnValue : 1;
	};


	gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.GameEditor_Actor_TA.EditorMoveToLocation", [this](ActorWrapper cw, void* params, std::string eventName) {
    auto* p = reinterpret_cast<pExecEditorMoveToLocaction*>(params);

    const float diag = 7950;
	float diagReduction = 0;
	diagBound = 7950;
    const float ybuff = 5050;
    const float xbuff = 4026;
    const float zMin = 150;
	const float upperRampSize = 450;
	const float zMax = 2044 - upperRampSize;
    const float baseRampDepth = 150;
    const float baseBoundaryShrink = 150;
    const float ceilingScale = upperRampSize / baseRampDepth;

    cw.SetbCollideWorld(0);

    p->NewLocation.Z = std::clamp(p->NewLocation.Z, 0.0f, 2005.0f);

    currentXBound = xbuff;
    currentYBound = ybuff;
    
    if (p->NewLocation.Z < zMin || p->NewLocation.Z > zMax) {
        isCeiling = (p->NewLocation.Z > zMax);
        float rampDepth = isCeiling ? baseRampDepth * ceilingScale : baseRampDepth;
        float boundaryShrink = isCeiling ? baseBoundaryShrink * ceilingScale : baseBoundaryShrink;

        t = isCeiling
            ? (p->NewLocation.Z - zMax) / rampDepth
            : (zMin - p->NewLocation.Z) / rampDepth;
        t = std::clamp(t, 0.0f, 1.0f);

        float maxShrinkRatio = (xbuff - boundaryShrink) / xbuff;
        float circleFactor = sqrt(1.0f - t * t * (1 - maxShrinkRatio * maxShrinkRatio));
		//LOG("t: {}, maxShrinkRatio: {}, circleFactor: {}", t, maxShrinkRatio, circleFactor);
		//LOG("t factor : {}", t);
        currentXBound = xbuff * circleFactor;
        currentYBound = ybuff * circleFactor;
		diagBound = diag * circleFactor;
		//LOG("currentXBound: {}, currentYBound: {}", currentXBound, currentYBound);
    }

    
    auto clampToDiagonal = [](float& x, float& y, float A, float B, float C) {
        float d = (A * x + B * y + C) / (A * A + B * B);
        x = x - A * d;
        y = y - B * d;
    };

    // Topleft
    if (diagBound < p->NewLocation.X + p->NewLocation.Y) {
        clampToDiagonal(p->NewLocation.X, p->NewLocation.Y, 1, 1, -diagBound);
    }
    // downright
    else if (p->NewLocation.X + p->NewLocation.Y < -diagBound) {
        clampToDiagonal(p->NewLocation.X, p->NewLocation.Y, 1, 1, diagBound);
    }
    // Topright
    else if ((p->NewLocation.X - p->NewLocation.Y) > diagBound) {
        clampToDiagonal(p->NewLocation.X, p->NewLocation.Y, 1, -1, -diagBound);
    }
    // downleft
    else if ((p->NewLocation.Y - p->NewLocation.X) > diagBound) {
        clampToDiagonal(p->NewLocation.X, p->NewLocation.Y, 1, -1, diagBound);
    }

    // apply bounding
    p->NewLocation.X = std::clamp(p->NewLocation.X, -currentXBound, currentXBound);
    p->NewLocation.Y = std::clamp(p->NewLocation.Y, -currentYBound, currentYBound);

    // Debug 
    //LOG("Final Position: X={}, Y={}, Z={}", p->NewLocation.X, p->NewLocation.Y, p->NewLocation.Z);
});
	
	gameWrapper->HookEventWithCallerPost<ActorWrapper>("Function TAGame.GameEditor_Actor_TA.EditorSetRotation", [this](ActorWrapper cw, void* params, std::string eventName) {
		if (editingVariances) {
			if (!cw || cw.IsNull()) {
				LOG("Server not found");
				return;
			}

			Vector loc = cw.GetLocation();
			Rotator rot1 = checkForClamping(loc, currentRotation);
			if (!lockRotation) {
				

				Rotator rot = cw.GetRotation();
				LOG("Rotation - Pitch: {}, Yaw: {}, Roll: {}", rot.Pitch, rot.Yaw, rot.Roll);
				rot.Yaw += rotationToApply.Yaw;
				if (rotationToApply.Pitch == 0) {
					rot.Pitch = currentRotation.Pitch;
				}
				else {
					rot.Pitch = rotationToApply.Pitch;
				}
				//testing, uncomment
				rot.Roll += rotationToApply.Roll;

				//rot += rotationToApply;
				rotationToApply = { 0,0,0 };
				cw.SetRotation(rot);
				currentRotation = rot;
				currentLocation = loc;
				if(localRotation.Roll != 0 && localRotation.Pitch != 0 && localRotation.Yaw != 0) {
					cw.SetRotation(localRotation);
					localRotation = { 0,0,0 };
					currentRotation = localRotation;
				}
				
				
				if (rot1.Yaw != 0 && rot.Pitch != 0 && rot.Roll != 0) {
					//Pitch, Yaw, Roll;
					cw.SetRotation({ rot1.Pitch,rot1.Yaw,rot1.Roll });//currentRotation.Pitch
				}


			}
		}
		});

	gameWrapper->HookEventWithCallerPost<ActorWrapper>("Function TAGame.PlayerController_TA.GetRotateActorCameraOffset",[this](ActorWrapper cw, void* params, std::string eventName) {
		if (editingVariances && !lockRotation) {
			if (!cw || cw.IsNull()) {
				LOG("Server not found");
				return;
			}
			
			//test

			if (clampVal != 5) {
				Rotator rot = gameWrapper->GetCamera().GetRotation();

				rotationToApply.Pitch = rot.Pitch;
			}

		}
	});
	
	
	//TAGame.PlayerController_TA.EventTrainingEditorActorModified
	gameWrapper->HookEvent("Function TAGame.PlayerController_TA.EventTrainingEditorActorModified",
		[this](std::string eventName) {
			if (editingVariances) {
				//LOG("Training editor actor modified");
				//checkForR1Press();
				/*checkForButtonPress(4);
				checkForButtonPress(5);*/  //when not connected, it fucks up
				//checkForButtonPress(6);

			
				if (!lockRotation) {
					//if (GetAsyncKeyState('R') & 0x8000) {
					//	rotationToApply.Pitch += 75;
					//	//LOG("Pitch increased");
					//}
					//if (GetAsyncKeyState('C') & 0x8000) {
					//	rotationToApply.Pitch -= 75;
					//	//LOG("Pitch decreased");
					//}

					if (GetAsyncKeyState('R') & 0x8000) { // Pitch up
						
							ApplyLocalPitch( 500.0f); // GOTTA BE tested
							LOG("applying localPitch");
					}
					if (GetAsyncKeyState('C') & 0x8000) { // Pitch down
						ApplyLocalPitch(-500.0f); // GOTTA BE tested
						LOG("applying localPitch");
					}
					if (GetAsyncKeyState('Q') & 0x8000) {
						rotationToApply.Roll -= 75;
						//LOG("Roll decreased");
					}
					if (GetAsyncKeyState('E') & 0x8000) {
						rotationToApply.Roll += 75;
						//LOG("Roll increased");
					}
				}
				if (GetAsyncKeyState('2') & 0x8000) {
					tempBoostAmount++;
					if (tempBoostAmount > boostMax) {
						tempBoostAmount = boostMax;
					}
					LOG("boost increased to {}", tempBoostAmount);
					//LOG("Roll increased");
				}
				if (GetAsyncKeyState('1') & 0x8000) {
					tempBoostAmount--;
					if (tempBoostAmount < boostMin) {
						tempBoostAmount = boostMin;
					}
					LOG("boost decreased to {}", tempBoostAmount);
				}
				if (GetAsyncKeyState('4') & 0x8000) {
					tempStartingVelocity++;
					if (tempStartingVelocity > maxVelocity) {
						tempStartingVelocity = maxVelocity;
					}
					LOG("starting velocity increased to {}", tempStartingVelocity);
				}
				if (GetAsyncKeyState('3') & 0x8000) {
					tempStartingVelocity--;
					if (tempStartingVelocity < minVelocity) {
						tempStartingVelocity = minVelocity;
					}
					LOG("starting velocity decreased to {}", tempStartingVelocity);
				}
				/*if (GetAsyncKeyState('4') & 0x8000) {
					tempStartingVelocityMin++;
					if (tempStartingVelocityMin > maxVelocity) {
						tempStartingVelocityMin = maxVelocity;
					}
					LOG("starting velocity increased to {}", tempStartingVelocityMin);
				}
				if (GetAsyncKeyState('3') & 0x8000) {
					tempStartingVelocityMin--;
					if (tempStartingVelocityMin < 0) {
						tempStartingVelocityMin = minVelocity;
					}
					LOG("starting velocity decreased to {}", tempStartingVelocityMin);
				}*/
				/*if (GetAsyncKeyState('8') & 0x8000) {
					tempStartingVelocityMax++;
					if (tempStartingVelocityMax > maxVelocity) {
						tempStartingVelocityMax = maxVelocity;
					}
					LOG("starting velocity increased to {}", tempStartingVelocityMax);
				}
				if (GetAsyncKeyState('7') & 0x8000) {
					tempStartingVelocityMax--;
					if (tempStartingVelocityMax < tempStartingVelocityMin) {
						tempStartingVelocityMax = tempStartingVelocityMin;
					}
					LOG("starting velocity decreased to {}", tempStartingVelocityMax);
				}*/
				
				
			}
		}
	);

	//gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.GFxData_TrainingModeBrowser_TA.GetLocalTrainingFiles",
	//	[this](ActorWrapper cw, void* params, std::string eventName) {
	//		//print out all local training packs

	//		LOG("GetLocalTrainingFiles called");
	//		if (!cw) {
	//			LOG("Caller is invalid");
	//			return;
	//		}
	//		TrainingEditorWrapper tw(cw.memory_address);
	//		if (tw.IsNull()) {
	//			LOG("Failed to get TrainingEditorWrapper");
	//			return;
	//		}
	//		GameEditorSaveDataWrapper data = tw.GetTrainingData();
	//		TrainingEditorSaveDataWrapper td = data.GetTrainingData();

	//		
	//	}
	//);


	

		
}


bool VersatileTraining::changeCarSpawnRotation() {
	auto serv = gameWrapper->GetGameEventAsServer();
	if (!serv) return 0;
	auto car = serv.GetGameCar();
	Rotator rot = car.GetRotation();
	LOG("Car rotation: {}", rot.Pitch);
	LOG("Car rotation: {}", rot.Yaw);
	LOG("Car rotation: {}", rot.Roll);
	rot.Yaw += 23036;
	car.SetCarRotation(rot);

	Rotator newRot = car.GetRotation();
	LOG("Car rotation: {}", newRot.Pitch);
	LOG("Car rotation: {}", newRot.Yaw);
	LOG("Car rotation: {}", newRot.Roll);
	LOG("Car rotation changed");
	return 1;
}
TrainingEditorWrapper VersatileTraining::GetTrainingEditor() {
	auto serv = gameWrapper->GetGameEventAsServer();
	if (!serv) return { 0 };
	return TrainingEditorWrapper(serv.memory_address);
}

void VersatileTraining::getTrainingData(ActorWrapper cw, void* params, std::string eventName) {



	auto tw = ((TrainingEditorWrapper)cw.memory_address);
	GameEditorSaveDataWrapper data = tw.GetTrainingData();
	TrainingEditorSaveDataWrapper td = data.GetTrainingData();
	LOG("Training pack code: {}", td.GetCode().ToString());
	std::string name = td.GetTM_Name().ToString();
	LOG("Training pack name", name);
	/*GetCreatorName();
	GetDescription();*/
	LOG("Training pack type: {}", td.GetType());
	LOG("Training pack difficulty: {}", td.GetDifficulty());
	LOG("Training pack creator name: {}", td.GetCreatorName().ToString());
	LOG("Training pack description: {}", td.GetDescription().ToString());
	LOG("training pack created at: {}", td.GetCreatedAt());
	int totalRounds = tw.GetTotalRounds();
	LOG("Training num rounds: {}", totalRounds);

	int currentShot = tw.GetActiveRoundNumber();
	LOG("Training current shot : {}", currentShot);

	std::string code = td.GetCode().ToString();

	bool found = false;
	for (auto& [key, value] : trainingData) {
		if (value.name == name) {
			currentTrainingData = value;
			LOG("setting active boost amount to {}", currentTrainingData.boostAmounts[currentShot]);
			tempBoostAmount = currentTrainingData.boostAmounts[currentShot];
			tempStartingVelocity = currentTrainingData.startingVelocity[currentShot];
			LOG("setting active starting velocity to {}", tempStartingVelocity);
			currentTrainingData.currentEditedShot = currentShot;
			freezeCar = currentTrainingData.freezeCar[currentShot];
			if (freezeCar) {
				LOG("car is frozen");
			}
			else {
				LOG("car is not frozen");
			}
			if (tempBoostAmount == 101) {
				cvarManager->executeCommand("sv_training_limitboost -1");
			}
			else {
				cvarManager->executeCommand("sv_training_limitboost " + std::to_string(tempBoostAmount));
			}
			found = true;
		}
	}
	if (!found) {
		LOG("didn't find this traini pack in training data");
		currentTrainingData.initCustomTrainingData(totalRounds, name);
		currentTrainingData.customPack = false;
		cvarManager->executeCommand("sv_training_limitboost -1");
	}
	//auto foundCode =  trainingData.find(code);
	//if (foundCode != trainingData.end()) {
	//	LOG("Training data found: {}", code);
	//	foundCode->second.name = td.GetTM_Name().ToString();
	//	foundCode->second.numShots = totalRounds;
	//	LOG("loaded training name: {}", foundCode->second.name);
	//	cvarManager->executeCommand("sv_training_enabled 1");
	//	cvarManager->executeCommand("sv_training_limitboost " + std::to_string(foundCode->second.boostAmounts[0]));
	//	//sv_training_player_velocity (1700, 1900);
	//	//cvarManager->executeCommand("sv_training_player_velocity ("+ std::to_string(foundCode->second.startingVelocityMin[0]) + "," + std::to_string(foundCode->second.startingVelocityMax[0]) + ")");

	//}
	//else {
	//	/*LOG("Training data not found: {}", code);
	//	cvarManager->executeCommand("sv_training_enabled 0");
	//	cvarManager->executeCommand("sv_training_limitboost -1");*/
	//	//overriden for testing
	//	//cvarManager->executeCommand("sv_training_enabled 1");
	//	if (tempBoostAmount == 101) {
	//		cvarManager->executeCommand("sv_training_limitboost -1");
	//	}
	//	else {
	//		cvarManager->executeCommand("sv_training_limitboost " + std::to_string(tempBoostAmount));
	//		//cvarManager->executeCommand("sv_training_player_velocity (" + std::to_string(tempStartingVelocityMin) + "," + std::to_string(tempStartingVelocityMax) + ")");
	//	}

	//}
	
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
	for (auto& [key, value] : trainingData) {
		shiftVelocitiesToPositive(value.startingVelocity);
	}
	SaveCompressedTrainingData(trainingData, saveFilePath);
	CleanUp();
}

void VersatileTraining::CleanUp() {
	for (auto& controller : controllers) {
		if (controller) {
			controller->Unacquire(); // Unacquire before release
			controller->Release();   // Release the DirectInput device
			controller = nullptr;
		}
	}
	controllers.clear(); // Clear the vector
	LOG("All controllers cleaned up.");

	if (dinput) {
		dinput->Release();
		dinput = nullptr;
		LOG("DirectInput object released.");
	}
}






void VersatileTraining::Render(CanvasWrapper canvas) {
	if (!editingVariances) return;

	LinearColor colors;
	colors.R = 255;
	colors.G = 255;
	colors.B = 0;
	colors.A = 255;
	canvas.SetColor(colors);

	canvas.SetPosition(Vector2F{ 0.0, 0.0 });
	canvas.DrawString("Boost Amount: " + std::to_string(tempBoostAmount), 2.0, 2.0,false);
	canvas.SetPosition(Vector2F{ 0.0, 20.0 });
	if (lockRotation) {
		canvas.DrawString("Car rotation locked, press X to unlock", 2.0, 2.0, false);
	}
	else {
		canvas.DrawString("Car rotation unlocked, press X to lock", 2.0, 2.0, false);
	}
	canvas.SetPosition(Vector2F{ 0.0, 40.0 });
	if (freezeCar) {
		
		canvas.DrawString("Car frozen, press F to unfreeze", 2.0, 2.0, false);
	}
	else {

		canvas.DrawString("Car unfrozen, press F to freeze", 2.0, 2.0, false);
	}
	canvas.SetPosition(Vector2F{ 0.0, 60.0 });
	//canvas.DrawString("Starting Velocity: (" + std::to_string(tempStartingVelocityMin) + "," + std::to_string(tempStartingVelocityMax) + ")", 2.0, 2.0, false);

	//tempStartingVelocity
	canvas.DrawString("Starting Velocity: " + std::to_string(tempStartingVelocity), 2.0, 2.0, false);

	//canvas.SetPosition(20, 20);
	//canvas.SetColor(255, 255, 255, 255);
	//canvas.DrawString("Hello world!", 1, 1);
}