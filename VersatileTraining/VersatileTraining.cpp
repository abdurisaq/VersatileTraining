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
	registerNotifiers();
	gameWrapper->RegisterDrawable([this](CanvasWrapper canvas) {
		Render(canvas);
	});

	middleMouseIndex = gameWrapper->GetFNameIndexByString("MiddleMouseButton");
	gameWrapper->HookEvent("Function Engine.GameViewportClient.Tick",
		std::bind(&VersatileTraining::onTick, this, std::placeholders::_1));


	initializeCallBacks();

	std::filesystem::path myDataFolder = gameWrapper->GetDataFolder() / "VersatileTraining";
	saveFilePath = myDataFolder / "packs.txt";

	trainingData = LoadCompressedTrainingData(saveFilePath);
	for (auto& [key, value] : trainingData) {
		shiftVelocitiesToNegative(value.startingVelocity);
		shiftGoalBlockerToNegative(value.goalBlockers);
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
void VersatileTraining::registerNotifiers() {

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
	cvarManager->registerNotifier("unlockCar", [this](std::vector<std::string> args) {

		if (lockRotation) {
			LOG("car unlocked");
		}
		else {
			LOG("car locked");
		}
		lockRotation = !lockRotation;


		}, "unlock car", PERMISSION_ALL);

	cvarManager->setBind("X", "unlockCar");
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
				LOG("Shot {}: Boost Amount: {}, Starting Velocity: {}, Freeze Car: {}. goal blocker x1 {}, z1 {} x2 {}, z2 {}", i, value.boostAmounts[i], value.startingVelocity[i], 
					static_cast<int>(value.freezeCar[i]),static_cast<int>(value.goalBlockers[i].first.X), static_cast<int>(value.goalBlockers[i].first.Z), 
					static_cast<int>(value.goalBlockers[i].second.X), static_cast<int>(value.goalBlockers[i].second.X));
				LOG("goal anchors first: {}, second: {}", value.goalAnchors[i].first ? "true" : "false", value.goalAnchors[i].second ? "true" : "false");
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



	cvarManager->registerNotifier("editGoalBlocker", [this](std::vector<std::string> args) {
		
		if (!editingGoalBlocker) {
			LOG("Editing goal blocker");
		}
		else{
			LOG("Not editing goal blocker");
		}


		if (currentTrainingData.customPack && goalBlockerEligbleToBeEdited ) {//&& gameWrapper->IsInCustomTraining()
			LOG("flipping editing goal blocker to {}", editingGoalBlocker ? "true" : "false");
			editingGoalBlocker = !editingGoalBlocker;
		}
		else {
			LOG("not in a custom pack and picked shot, cant edit goal blocker");
		}

		}, "toggling editting goal blocker", PERMISSION_ALL);
	cvarManager->setBind("G", "editGoalBlocker");


	cvarManager->registerNotifier("spawnBot", [this](std::vector<std::string> args) {
		if (botSpawnedTest) {
			LOG("bot already spawned");
			return;
		}
		if(currentShotRecording == nullptr) return;
		if (currentShotRecording->inputs.size() == 0) {
			LOG("no inputs to play");
			return;
		}
		/*if (!canSpawnBot) {
			LOG("not in custom training");
			return;
		}*/
		auto server = gameWrapper->GetCurrentGameState();
		if (!server) return;
		frame = 0;
		ProfileCameraSettings settings = gameWrapper->GetSettings().GetCameraSettings();
		std::vector<std::pair<std::string, std::string>> bindings = gameWrapper->GetSettings().GetAllPCBindings();
		for (auto& binding : bindings) {
			LOG("binding: {} : {}", binding.first, binding.second);
		}
		auto car = server.GetGameCar();
		Vector carLoc = car.GetLocation();
		Rotator carRot = car.GetRotation();
		LOG(" car location: {:.7f}, {:.7f}, {:.7f}", car.GetLocation().X, car.GetLocation().Y, car.GetLocation().Z);
		LOG(" rotation : {}, {}, {}", car.GetRotation().Pitch, car.GetRotation().Yaw, car.GetRotation().Roll);
		server.DestroyCars();
		gameWrapper->SetTimeout([this,settings](GameWrapper* gw) {
			auto server = gw->GetCurrentGameState();

			server.SpawnBot(this->currentShotRecording->carBody, "testplayers");

			auto car = server.GetGameCar();

			PlayerControllerWrapper player_controller = server.GetLocalPrimaryPlayer();
			car.SetPlayerController(player_controller);
			player_controller.SetCar(car);
			car.GetAIController().DoNothing();
			
			ViewTarget new_view_target;
			new_view_target.Controller = (void*)player_controller.memory_address;
			new_view_target.Target = (void*)car.memory_address;
			new_view_target.PRI = (void*)player_controller.GetPRI().memory_address;

			CameraWrapper cam = gw->GetCamera();
			cam.SetViewTarget(new_view_target);
			cam.SetFlyCamBallTargetMode();
			cam.SetFocusActor("ball");



			}, 0.05f);
		gameWrapper->SetTimeout([this, settings](GameWrapper* gw) { //needs a timeout before setting boost for some reason, if set immediately, crashes game
			auto server = gw->GetCurrentGameState();
			if (!server) return;

			auto car = server.GetGameCar();
			BoostWrapper boost = car.GetBoostComponent();
			//car.SetbDriving(false);
			boost.SetUnlimitedBoost2(true);
			car.GetPRI().SetUserCarPreferences(currentShotRecording->settings.DodgeInputThreshold, currentShotRecording->settings.SteeringSensitivity, currentShotRecording->settings.AirControlSensitivity);
			botSpawnedTest = true;
			/*if (freezeForShot) {
				car.SetLocation(currentShotRecording->initialState->location);
			}*/
			CameraWrapper cameraWrapper = gw->GetCamera();
			cameraWrapper.SetCameraSettings(settings);
			/*car.SetLocation(currentShotRecording->initialState->location);
			car.SetRotation(currentShotRecording->initialState->rotation);*/
			
			}, 0.5f);

		gameWrapper->SetTimeout([this, settings](GameWrapper* gw) {
			canStartPlayback = true;
			auto server = gw->GetCurrentGameState();
			if (!server) return;

			auto car = server.GetGameCar();
			RBState state = car.GetRBState();
			LOG("=== RBState ===");
			LOG("Location: ({}, {}, {})", state.Location.X, state.Location.Y, state.Location.Z);
			LOG("LinearVelocity: ({}, {}, {})", state.LinearVelocity.X, state.LinearVelocity.Y, state.LinearVelocity.Z);
			LOG("AngularVelocity: ({}, {}, {})", state.AngularVelocity.X, state.AngularVelocity.Y, state.AngularVelocity.Z);
			LOG("Quaternion: ({}, {}, {}, {})", state.Quaternion.W, state.Quaternion.X, state.Quaternion.Y, state.Quaternion.Z);
			LOG("Time: {}", state.Time);
			LOG("bSleeping: {}", state.bSleeping ? "true" : "false");
			LOG("bNewData: {}", state.bNewData ? "true" : "false");
			LOG("frozen: {}", car.GetbFrozen());
			LOG("sleeping: {}", car.GetbDisableSleeping());
			LOG(" moving : {}", car.GetbIsMoving());
			LOG("movable : {}", car.GetbMovable());
			car.SetFrozen(false);
			car.SetbDisableSleeping(true);
			car.SetbFrozen(false);
			car.SetbDriving(true);
			GfxDataTrainingWrapper test= gameWrapper->GetGfxTrainingData();
			ServerWrapper sw = gw->GetGameEventAsServer();
			if (!sw) return;
			GameEditorWrapper training = GameEditorWrapper(sw.memory_address);
			LOG("awake: {} ", car.GetbRigidBodyWasAwake());
			RBState playerState;
			playerState.Location = currentShotRecording->initialState->location;
			playerState.Quaternion= RotatorToQuat(currentShotRecording->initialState->rotation);
			car.SetPhysicsState(startState);
		//car.SetbDriving(false);


		}, 0.5f);
		gameWrapper->SetTimeout([this, settings](GameWrapper* gw) {
				if (!startPlayback && canStartPlayback) {
					frame = 0;

					//roundStarted = true;
					startPlayback = true;
					canStartPlayback = false;
				}
			}, 0.5f);//0.5 1 

		}, "spawn bot in custom training", PERMISSION_ALL);
	cvarManager->setBind("M", "spawnBot");

	cvarManager->registerNotifier("startRecording", [this](std::vector<std::string> args) {
		if(botSpawnedTest) return;
		if (!startRecording) {
			if (botSpawnedTest || playForNormalCar) {
				botSpawnedTest = false;
				playForNormalCar = false;
			}
			currentShotRecording = std::make_shared<ShotRecording>();
			currentShotRecording->carBody = gameWrapper->GetLocalCar().GetLoadoutBody();
			currentShotRecording->settings = gameWrapper->GetSettings().GetGamepadSettings();
			

			LOG("settings controllerDeadzone {}, DodgeInputThreshold {}, SteeringSensitivity {} , AirControlSensitivity {}", currentShotRecording->settings.ControllerDeadzone, currentShotRecording->settings.DodgeInputThreshold, currentShotRecording->settings.SteeringSensitivity, currentShotRecording->settings.AirControlSensitivity);
			// Record initial state
			auto car = gameWrapper->GetLocalCar();
			currentShotRecording->initialState = std::make_shared<ShotRecording::InitialState>();
			currentShotRecording->initialState->velocity = car.GetVelocity();
			currentShotRecording->initialState->location = car.GetLocation();
			currentShotRecording->initialState->rotation = car.GetRotation();

			startState = car.GetRBState();
			

			primedToStartRecording = true;
		}
		}, "start recording", PERMISSION_ALL);
	cvarManager->setBind("N", "startRecording");

	cvarManager->registerNotifier("playForNormalCar", [this](std::vector<std::string> args) {
		playForNormalCar = !playForNormalCar;
		canStartPlayback = true;
		}, "start recording", PERMISSION_ALL);
	cvarManager->setBind("V", "playForNormalCar");


	cvarManager->registerNotifier("startround", [this](std::vector<std::string> args) {

		auto car = gameWrapper->GetLocalCar();
		
		if (!startPlayback&& canStartPlayback) {
			frame = 0;

			//roundStarted = true;
			startPlayback = true;
			canStartPlayback = false;
		}
		
		
		
		}, "start recording", PERMISSION_ALL);
	cvarManager->setBind("K", "startround");

	cvarManager->registerNotifier("startboost", [this](std::vector<std::string> args) {

		HWND hwnd = FindWindowA("LaunchUnrealUWindowsClient", "Rocket League (64-bit, DX11, Cooked)");
		PostMessage(hwnd, WM_KEYUP, 'D', 0);//'W'

		}, "start recording", PERMISSION_ALL);
	cvarManager->setBind("I", "startboost");
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
			currentTrainingData.customPack = true;
			LOG("setting active boost amount to {}", currentTrainingData.boostAmounts[currentShot]);
			tempBoostAmount = currentTrainingData.boostAmounts[currentShot];
			tempStartingVelocity = currentTrainingData.startingVelocity[currentShot];
			LOG("setting active starting velocity to {}", tempStartingVelocity);
			currentTrainingData.currentEditedShot = currentShot;
			freezeCar = currentTrainingData.freezeCar[currentShot];
			std::pair<Vector,Vector> blockerToAssign = currentTrainingData.goalBlockers[currentShot];

			//goalAnchors.first = !(blockerToAssign.first.X == 0.f || blockerToAssign.first.Z == 0.f) ;//&& (blockerToAssign.first != Vector{ 0,0,0 })
			//goalAnchors.second = !(blockerToAssign.second.X == 0.f || blockerToAssign.second.Z == 0.f);//&& (blockerToAssign.second != Vector{ 0,0,0 })
			goalAnchors = currentTrainingData.goalAnchors[currentShot];
			goalBlockerPos = currentTrainingData.goalBlockers[currentShot];
			LOG("pulled goalblocker, x1:{}, z1:{} x2:{} z2{}. setting anchor first to : {}, and send to : {}", goalBlockerPos.first.X, goalBlockerPos.first.Z, goalBlockerPos.second.X, goalBlockerPos.second.Z, goalAnchors.first ? "true" : "false", goalAnchors.second ? "true" : "false");

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
		if (td.GetCode().ToString().empty()) {
			currentTrainingData.customPack = true;
		}
		else {
			currentTrainingData.customPack = false;
		}
		cvarManager->executeCommand("sv_training_limitboost -1");
	}

	
}



void VersatileTraining::onUnload() {
	LOG("Unloading Versatile Training");
	for (auto& [key, value] : trainingData) {
		shiftVelocitiesToPositive(value.startingVelocity);
		shiftGoalBlockerToPositive(value.goalBlockers);
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

bool inRectangle(const std::pair<Vector, Vector>& goalBlockerPos, const Vector& ballLoc) {
	float minX = min(goalBlockerPos.first.X, goalBlockerPos.second.X);
	float maxX = max(goalBlockerPos.first.X, goalBlockerPos.second.X);

	float minZ = min(goalBlockerPos.first.Z, goalBlockerPos.second.Z);
	float maxZ = max(goalBlockerPos.first.Z, goalBlockerPos.second.Z);

	// Y is usually constant (the back wall), so we just care about X and Z for 2D plane
	return (ballLoc.X >= minX && ballLoc.X <= maxX &&
		ballLoc.Z >= minZ && ballLoc.Z <= maxZ);
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

	if (!gameWrapper->IsInCustomTraining())return;


	if(!currentTrainingData.customPack) return;
	if (!goalAnchors.first  || !goalAnchors.second) return;

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

		if (inRectangle(goalBlockerPos, ballPosition)) {
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


Vector GetForwardVectorFromRotator(const Rotator& rot) {
    float yawRad = rot.Yaw * (2.0f * PI / 65536.0f);
    float pitchRad = rot.Pitch * (PI / 32768.0f);

    float cosPitch = cosf(pitchRad);

    return Vector{
        cosPitch * cosf(yawRad), // X
        cosPitch * sinf(yawRad), // Y
        sinf(pitchRad)           // Z
    };
}

bool ProjectToPlaneY(Vector origin, Vector direction, float yPlane, Vector& outHit) {
	// get point by projecting onto Y = backWall
	if (fabs(direction.Y) < 1e-6) return false; //to make sure it doesnt fuck with 0

	float t = (yPlane - origin.Y) / direction.Y;
	if (t < 0) return false; // Behind the camera

	outHit = origin + direction * t;
	return true;
}

float Dot(const Vector2& a, const Vector2& b) {
	return a.X * b.X + a.Y * b.Y;
}

void DrawLineClippedByCircle(CanvasWrapper& canvas, const Vector2& a, const Vector2& b, const Vector2& circleCenter, float radius, const Vector& a3D, const Vector& b3D, CameraWrapper cam, RT::Frustum frust) {
	Vector2 ab = b - a;
	Vector2 ac = a - circleCenter;

	float A = Dot(ab, ab);
	float B = 2 * Dot(ac, ab);
	float C = Dot(ac, ac) - radius * radius;

	float discriminant = B * B - 4 * A * C;

	// No intersection: draw full line
	if (discriminant < 0.0f) {
		RT::Line full(a3D, b3D, RT::GetVisualDistance(canvas, frust, cam, a3D) * 1.5f);
		full.DrawWithinFrustum(canvas, frust);
		return;
	}

	// Compute intersection points (in screen space as t values)
	float sqrtDisc = std::sqrt(discriminant);
	float t1 = (-B - sqrtDisc) / (2 * A);
	float t2 = (-B + sqrtDisc) / (2 * A);

	t1 = std::clamp(t1, 0.0f, 1.0f);
	t2 = std::clamp(t2, 0.0f, 1.0f);

	Vector2 p1 = a + ab * t1;
	Vector2 p2 = a + ab * t2;

	Vector v3D = b3D - a3D;
	Vector p1_3D = a3D + v3D * t1;
	Vector p2_3D = a3D + v3D * t2;

	if (t1 > 0.0f) {
		RT::Line line1(a3D, p1_3D, RT::GetVisualDistance(canvas, frust, cam, a3D) * 1.5f);
		line1.DrawWithinFrustum(canvas, frust);
	}
	if (t2 < 1.0f) {
		RT::Line line2(p2_3D, b3D, RT::GetVisualDistance(canvas, frust, cam, b3D) * 1.5f);
		line2.DrawWithinFrustum(canvas, frust);
	}
}


float Distance(Vector2 a, Vector2 b) {
	float dx = b.X - a.X;
	float dy = b.Y - a.Y;
	return sqrt(dx * dx + dy * dy);
}
void VersatileTraining::Render(CanvasWrapper canvas) {
	if (editingVariances) {

		LinearColor colors;
		colors.R = 255;
		colors.G = 255;
		colors.B = 0;
		colors.A = 255;
		canvas.SetColor(colors);

		canvas.SetPosition(Vector2F{ 0.0, 0.0 });
		canvas.DrawString("Boost Amount: " + std::to_string(tempBoostAmount), 2.0, 2.0, false);
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

		canvas.DrawString("Starting Velocity: " + std::to_string(tempStartingVelocity), 2.0, 2.0, false);
	}
	else if (editingGoalBlocker) {
		CameraWrapper cam = gameWrapper->GetCamera();
		if (cam.IsNull()) return;

		Vector camLoc = cam.GetLocation();
		Vector forward = GetForwardVectorFromRotator(cam.GetRotation());
		RT::Frustum frust{ canvas, cam };

		Vector projectedPoint;
		if (ProjectToPlaneY(camLoc, forward, backWall, projectedPoint)) {
			// Draw projected point as a small cross
			Vector2 screenPoint = canvas.Project(projectedPoint);
			float crossSize = 5.0f;
			canvas.SetColor(255, 255, 255, 255);
			canvas.DrawLine(screenPoint - Vector2(crossSize, 0), screenPoint + Vector2(crossSize, 0), 1.0f);
			canvas.DrawLine(screenPoint - Vector2(0, crossSize), screenPoint + Vector2(0, crossSize), 1.0f);

			// Handle mouse input
			if (saveCursorPos) {
				LOG("current anchors first: {}, second: {}", goalAnchors.first ? "true" : "false", goalAnchors.second ? "true" : "false");
				LOG("current anchor firs : X {} Z {}, second : X {} Z {}", goalBlockerPos.first.X, goalBlockerPos.first.Z, goalBlockerPos.second.X, goalBlockerPos.second.Z);
				if (!goalAnchors.first) {
					goalBlockerPos.first = projectedPoint;
					saveCursorPos = false;
					rectangleMade = false;
					rectangleSaved = false;
					goalAnchors.first = true;
					LOG("adding to first anchor point : X {}, Z {}", goalBlockerPos.first.X, goalBlockerPos.first.Z);
				}
				else if(!goalAnchors.second){
					goalBlockerPos.second = projectedPoint;
					rectangleSaved = true;
					saveCursorPos = false;
					goalAnchors.second = true;

					LOG("first anchor point : X {}, Z {}, second anchor point : X {} Z {}", goalBlockerPos.first.X, goalBlockerPos.first.Z, goalBlockerPos.second.X, goalBlockerPos.second.Z);
				}
				else {
					LOG("this else call is being called");
					goalBlockerPos.first = projectedPoint;
					goalBlockerPos.second = {0, 0, 0};
					goalAnchors.first = true;
					goalAnchors.second = false;
					rectangleMade = false;
					saveCursorPos = false;
					rectangleSaved = false;
				}
				//eventaully add logic for second middle mouse press to keep the goal blocker seperate from mouse
			}
			else if (goalAnchors.first && !goalAnchors.second) {
				
				goalBlockerPos.second = projectedPoint;
				//LOG("updating second anchor point : X {}, Z {}", goalBlockerPos.second.X, goalBlockerPos.second.Z);
				rectangleMade = true;

			}if (goalAnchors.first && goalAnchors.second) {
				rectangleMade = true;
			}
			

			// If we have both points, draw rectangle on the goal plane
			if (rectangleMade) {
				Vector topLeft(max(goalBlockerPos.first.X, goalBlockerPos.second.X), backWall, max(goalBlockerPos.first.Z, goalBlockerPos.second.Z));
				Vector topRight(min(goalBlockerPos.first.X, goalBlockerPos.second.X), backWall, max(goalBlockerPos.first.Z, goalBlockerPos.second.Z));
				Vector bottomLeft(max(goalBlockerPos.first.X, goalBlockerPos.second.X), backWall, min(goalBlockerPos.first.Z, goalBlockerPos.second.Z));
				Vector bottomRight(min(goalBlockerPos.first.X, goalBlockerPos.second.X), backWall, min(goalBlockerPos.first.Z, goalBlockerPos.second.Z));

				canvas.SetColor(0, 255, 0, 255); // Green

				RT::Line lineTop(topLeft, topRight, RT::GetVisualDistance(canvas, frust, cam, topLeft) * 1.5f);
				RT::Line lineRight(topRight, bottomRight, RT::GetVisualDistance(canvas, frust, cam, topRight) * 1.5f);
				RT::Line lineBottom(bottomRight, bottomLeft, RT::GetVisualDistance(canvas, frust, cam, bottomRight) * 1.5f);
				RT::Line lineLeft(bottomLeft, topLeft, RT::GetVisualDistance(canvas, frust, cam, bottomLeft) * 1.5f);

				lineTop.DrawWithinFrustum(canvas, frust);
				lineRight.DrawWithinFrustum(canvas, frust);
				lineBottom.DrawWithinFrustum(canvas, frust);
				lineLeft.DrawWithinFrustum(canvas, frust);

			}
		}

		// Optional: Draw center screen cursor
		Vector2 screenSize = canvas.GetSize();
		Vector2 center(screenSize.X / 2, screenSize.Y / 2);
		canvas.SetColor(255, 255, 255, 255);
		float length = 10.0f;
		canvas.DrawLine(Vector2(center.X - length, center.Y), Vector2(center.X + length, center.Y), 1.5f);
		canvas.DrawLine(Vector2(center.X, center.Y - length), Vector2(center.X, center.Y + length), 1.5f);


	}

	//if (!gameWrapper->IsInCustomTraining()) return;

	if (!(goalAnchors.first && goalAnchors.second)) return;
	

	CameraWrapper camera = gameWrapper->GetCamera();
	if (camera.IsNull()) return;

	RT::Frustum frustum{ canvas, camera };
	ServerWrapper gameState = gameWrapper->GetCurrentGameState();
	BallWrapper ball = gameState.GetBall();
	if (ball.IsNull()) return;

	Vector2 ballScreenPos = canvas.Project(ball.GetLocation());
	//float ballRadius = canvas.Project(ball.GetLocation() + Vector(92.75f, 0, 0)).Dist(ballScreenPos); // 92.75 uu = ball radius in RL
	float ballRadius = Distance(ballScreenPos, canvas.Project(ball.GetLocation() + Vector(92.75f, 0, 0)));
	// Goal blocker corner definitions
	Vector topLeft(max(goalBlockerPos.first.X, goalBlockerPos.second.X), backWall, max(goalBlockerPos.first.Z, goalBlockerPos.second.Z));
	Vector topRight(min(goalBlockerPos.first.X, goalBlockerPos.second.X), backWall, max(goalBlockerPos.first.Z, goalBlockerPos.second.Z));
	Vector bottomLeft(max(goalBlockerPos.first.X, goalBlockerPos.second.X), backWall, min(goalBlockerPos.first.Z, goalBlockerPos.second.Z));
	Vector bottomRight(min(goalBlockerPos.first.X, goalBlockerPos.second.X), backWall, min(goalBlockerPos.first.Z, goalBlockerPos.second.Z));

	// Project all points to 2D
	Vector2 tl2d = canvas.Project(topLeft);
	Vector2 tr2d = canvas.Project(topRight);
	Vector2 bl2d = canvas.Project(bottomLeft);
	Vector2 br2d = canvas.Project(bottomRight);

	canvas.SetColor(0, 255, 0, 255); // Green

	DrawLineClippedByCircle(canvas, tl2d, tr2d, ballScreenPos, ballRadius, topLeft, topRight, camera, frustum);
	DrawLineClippedByCircle(canvas, tr2d, br2d, ballScreenPos, ballRadius, topRight, bottomRight, camera, frustum);
	DrawLineClippedByCircle(canvas, br2d, bl2d, ballScreenPos, ballRadius, bottomRight, bottomLeft, camera, frustum);
	DrawLineClippedByCircle(canvas, bl2d, tl2d, ballScreenPos, ballRadius, bottomLeft, topLeft, camera, frustum);




}

