#include "pch.h"
#include "versatileTraining.h"

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
		editingGoalBlocker = false;
		});

	//TAGame.TrainingEditorMetrics_TA.TrainingEditorExit save when this is called
	//
	//TAGame.GameEvent_TrainingEditor_TA.GetTrainingMetrics
	gameWrapper->HookEventWithCallerPost<ActorWrapper>("Function TAGame.TrainingEditorMetrics_TA.TrainingEditorExit", [this](ActorWrapper cw, void* params, std::string eventName) {
		currentTrainingData.currentEditedShot = -1;
		editingGoalBlocker = false;
		goalBlockerEligbleToBeEdited = false;
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

			LOG("saving num shots : {}", currentTrainingData.numShots);
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
		goalBlockerEligbleToBeEdited = true;
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
			LOG("amount of shots in found training data: {}", currentTrainingData.numShots);
			while (currentShot >= currentTrainingData.numShots) {
				LOG("adding shot");
				currentTrainingData.addShot();
				LOG("now this many shots in training data: {}", currentTrainingData.numShots);
			}
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


		LOG("currentShot: {}", currentShot);
		LOG("amount of shots in found training data: {}", currentTrainingData.numShots);
		while (currentShot >= currentTrainingData.numShots) {
			LOG("adding shot");
			currentTrainingData.addShot();
			LOG("now this many shots in training data: {}", currentTrainingData.numShots);
		}
		LOG("setting boost atmount to {}", currentTrainingData.boostAmounts[currentShot]);
		tempBoostAmount = currentTrainingData.boostAmounts[currentShot];
		LOG("setting starting velocity to {}", currentTrainingData.startingVelocity[currentShot]);
		tempStartingVelocity = currentTrainingData.startingVelocity[currentShot];
		currentTrainingData.currentEditedShot = currentShot;
		LOG("setting freeze car to {}", currentTrainingData.freezeCar[currentShot] ? "false" : "true");
		freezeCar = currentTrainingData.freezeCar[currentShot];
		//currentTrainingDataUsed->currentEditedShot = currentShot;



		});

	//TAGame.GFxData_TrainingModeEditor_TA.CreateRound
	gameWrapper->HookEventWithCallerPost<TrainingEditorWrapper>("Function TAGame.GFxData_TrainingModeEditor_TA.CreateRound", [this](TrainingEditorWrapper cw, void* params, std::string eventName) {
		currentTrainingData.addShot();


		});
	gameWrapper->HookEventWithCallerPost<TrainingEditorWrapper>("Function TAGame.GameEvent_TrainingEditor_TA.DeleteRound", [this](TrainingEditorWrapper cw, void* params, std::string eventName) {
		int shotToRemove = cw.GetActiveRoundNumber();  // Get this BEFORE deletion

		if (shotToRemove >= 0 && shotToRemove < currentTrainingData.numShots) {
			LOG("Removing shot: {}", shotToRemove);

			currentTrainingData.boostAmounts.erase(currentTrainingData.boostAmounts.begin() + shotToRemove);
			currentTrainingData.startingVelocity.erase(currentTrainingData.startingVelocity.begin() + shotToRemove);
			currentTrainingData.freezeCar.erase(currentTrainingData.freezeCar.begin() + shotToRemove);

			currentTrainingData.numShots--;
		}
		else {
			LOG("Invalid shot index: {}, numShots = {}", shotToRemove, currentTrainingData.numShots);
		}
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

		currentTrainingData.addShot(boostToCopy, startingVelocityToCopy, freezeToCopy);




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
		if (goalBlockerEligbleToBeEdited) {
			goalBlockerEligbleToBeEdited = false;
			editingGoalBlocker = false;
		}
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
						/*Vector loc2 = getClampChange(loc, rot);
						if (loc2.X != 0 || loc2.Y != 0 || loc2.Z != 0) {

							car.SetLocation(loc2);
						}*/
						float pitchRad = (rot.Pitch / 16201.0f) * (PI / 2);
						float yawRad = (rot.Yaw / 32768.0f) * PI;
						float z = sinf(pitchRad);
						float y = cosf(pitchRad) * sinf(yawRad);
						float x = cosf(pitchRad) * cosf(yawRad);
						Vector unitVector = { x,y,z };
						int velocity = currentTrainingData.startingVelocity[currentTrainingData.currentEditedShot];//getRandomNumber(tempStartingVelocityMin, tempStartingVelocityMax);//changed 
						startingVelocityTranslation = unitVector * velocity;
					}
					Vector loc2 = getClampChange(loc, rot);
					if (loc2.X != 0 || loc2.Y != 0 || loc2.Z != 0) {
						LOG("old location - X: {}, Y: {}, Z: {}", loc.X, loc.Y, loc.Z);
						LOG("new location - X: {}, Y: {}, Z: {}", loc2.X, loc2.Y, loc2.Z);
						car.SetLocation(loc2);
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
			Rotator rot = cw.GetRotation();
			Rotator rot1 = checkForClamping(loc, rot);//currentRotation
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
				if (localRotation.Roll != 0 && localRotation.Pitch != 0 && localRotation.Yaw != 0) {
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

	gameWrapper->HookEventWithCallerPost<ActorWrapper>("Function TAGame.PlayerController_TA.GetRotateActorCameraOffset", [this](ActorWrapper cw, void* params, std::string eventName) {
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
					

					if (GetAsyncKeyState('R') & 0x8000) { // Pitch up

						ApplyLocalPitch(500.0f); // GOTTA BE tested
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



			}
		}
	);


	gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.Ball_GameEditor_TA.EditingBegin", [this](ActorWrapper cw, void* params, std::string eventName) {
		if (goalBlockerEligbleToBeEdited) {
			goalBlockerEligbleToBeEdited = false;
			editingGoalBlocker = false;
		}
		});

	gameWrapper->HookEventWithCaller<ActorWrapper>("Function PlayerController_TA.Editing.StopEditing", [this](ActorWrapper cw, void* params, std::string eventName) {
		if (goalBlockerEligbleToBeEdited) {
			goalBlockerEligbleToBeEdited = false;
			editingGoalBlocker = false;
		}
		});

	//PlayerController_TA.Editing.StopEditing
	gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.CameraState_GameEditor_TA.UpdateFlyPOV", [this](ActorWrapper cw, void* params, std::string eventName) {
		if (editingGoalBlocker) {
			Rotator rot = gameWrapper->GetCamera().GetRotation();
			Vector loc = gameWrapper->GetCamera().GetLocation();
			bool rotChanged = false;
			if (!savedPastCamera) {
				previousLocBeforeGoalEdit = loc;
				previousRotBeforeGoalEdit = rot;
				savedPastCamera = true;
			}
			LOG("Camera - Pitch: {}, Yaw: {}, Roll: {}", rot.Pitch, rot.Yaw, rot.Roll);
			LOG("Camera - X: {}, Y: {}, Z: {}", loc.X, loc.Y, loc.Z);


			loc.Y = 4000.f;
			loc.X = 0.f;
			loc.Z = 321.f;

			float centerYaw = 16384.f;
			float maxYawRange = 7000.f; // (23384 - 9384) / 2
			float normalizedYawDist = std::abs(rot.Yaw - centerYaw) / maxYawRange;
			normalizedYawDist = std::clamp(normalizedYawDist, 0.f, 1.f);

			// Inverse taper: smallest at edges, larger toward center
			float extraPitch = (1.f - normalizedYawDist) * 600.f;
			float allowedPitch = 2400.f + extraPitch;

			if (rot.Pitch > allowedPitch) {
				rot.Pitch = allowedPitch;
				rotChanged = true;
			}
			else if (rot.Pitch < -allowedPitch) {
				rot.Pitch = -allowedPitch;
				rotChanged = true;
			}

			if (rot.Yaw < 9384.f) {
				rot.Yaw = 9384.f;
				rotChanged = true;
			}
			else if (rot.Yaw > 23384.f) {
				rot.Yaw = 23384.f;
				rotChanged = true;
			}
			
			gameWrapper->GetCamera().SetLocation(loc);
			if (rotChanged) {
				gameWrapper->GetCamera().SetRotation(rot);
			}
			
		}
		else if (!editingGoalBlocker && savedPastCamera) {
			LOG("applying previous camera location and rotation, and setting saved to false");
			gameWrapper->GetCamera().SetLocation(previousLocBeforeGoalEdit);
			gameWrapper->GetCamera().SetRotation(previousRotBeforeGoalEdit);
			previousLocBeforeGoalEdit = { 0, 0, 0 };
			previousRotBeforeGoalEdit = { 0, 0, 0 };
			savedPastCamera = false;
		}
		});

}