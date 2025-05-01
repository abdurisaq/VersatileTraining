#include <pch.h>
#include "src/core/VersatileTraining.h"

int VersatileTraining::getRandomNumber(int min, int max) {
    std::random_device rd;  // Non-deterministic random number generator
    std::mt19937 gen(rd()); // Mersenne Twister engine
    std::uniform_int_distribution<> distrib(min, max);
    return distrib(gen);
}

Rotator blendPitchRollClampSmooth(int pitch, int roll, int expectedPitch, int expectedRoll, int desiredYaw, float alpha = 1.0f) {
	float pitchWeight = std::abs(static_cast<float>(pitch)) / 16384.0f;
	pitchWeight = std::clamp(pitchWeight, 0.0f, 1.0f);

	float rollWeight = 1.0f - pitchWeight;

	int targetPitch = static_cast<int>(expectedPitch * pitchWeight + pitch * (1.0f - pitchWeight));
	int targetRoll = static_cast<int>(expectedRoll * rollWeight + roll * (1.0f - rollWeight));

	int blendedPitch = static_cast<int>(pitch + (targetPitch - pitch) * alpha);
	int blendedRoll = static_cast<int>(roll + (targetRoll - roll) * alpha);

	return Rotator{ blendedPitch, desiredYaw, blendedRoll };
}


Rotator VersatileTraining::checkForClamping(Vector loc, Rotator rot) {

	
	auto inThreshold = [](int value, int center, int threshold) {
		return value >= (center - threshold) && value <= (center + threshold);
		};

	auto mapToRoll = [this,loc,rot](float value, float a, float b) {
		//float end = 0;
		if (isCeiling ) {
			if (b > 0) {
				b = 32768;//for some reason using % didn't work
			}
			else if (b < 0) {
				b = -32768;
			}
		}

		return a + (b - a) * value;
		};
	
	auto cornerLine = [this](int x, int y) {
		return (diagBound -25) + x + y;
		};//7950
	auto cornerLine2 = [this](int x, int y) {
		return (diagBound - 25) + x - y;
		};
	int yaw = rot.Yaw % 65536;//currentRotation.Yaw % 65536;

	int roll = rot.Roll % 65536;
	int pitch = rot.Pitch % 65536;
	//int roll = abs(currentRotation.Roll % 65536);

	int north = 16384;
	int south = 49152;
	int east = 32768;
	int west = 1;
	int end = 65535;
	int expectedRoll1 = mapToRoll(t, south, 65535);
	int expectedRoll2 = mapToRoll(t, north, west);
	int expectedPitch = mapToRoll(t, 0, 16384);
	//LOG("expectedRoll: {}", expectedRoll1);
	//LOG("actual roll: {}", roll);
	int tolerance = 5000;
	bool yaw1 = inThreshold(yaw, 0, tolerance) || inThreshold(yaw, 65536, tolerance);
	bool yaw2 = inThreshold(yaw, east, tolerance) || inThreshold(yaw, -east, tolerance);
	bool yaw3 = inThreshold(yaw, south, tolerance) || inThreshold(yaw, -north, tolerance);
	bool yaw4 = inThreshold(yaw, north, tolerance) || inThreshold(yaw, -south, tolerance);

	// Roll conditions, should be mapped based on t value for the ramps.
	bool roll1 = inThreshold(roll, expectedRoll1, tolerance) || inThreshold(roll, -expectedRoll2, tolerance);//south
	bool roll2 = inThreshold(roll, expectedRoll2, tolerance) || inThreshold(roll, -expectedRoll1, tolerance);
	bool roll3 = inThreshold(roll, east, tolerance) || inThreshold(roll, -east, tolerance);
	bool pitch1 = inThreshold(pitch, 0, 2500);


	//diagonal values

	bool yaw5 = inThreshold(yaw, 8192, tolerance) || inThreshold(yaw, -57344, tolerance);
	bool yaw6 = inThreshold(yaw, 40960, tolerance) || inThreshold(yaw, -24576, tolerance);
	bool yaw7 = inThreshold(yaw, 24576, tolerance) || inThreshold(yaw, -40960, tolerance);
	bool yaw8 = inThreshold(yaw, 57344, tolerance) || inThreshold(yaw, -8192, tolerance);

	//summarize maybe into an easy to see loop later
	if (loc.Y > (currentYBound - 10)) {

		if (yaw1  && (roll1))
		{

			LOG("clamped to orange back wall1");
			clampVal = 1;
			return Rotator{ pitch,1, expectedRoll1 };
		}
		else if ( yaw2  && (roll2) )
		{
			LOG("clamped to orange back wall2");
			clampVal = 1;
			return Rotator{ pitch,32768 ,expectedRoll2 };
		}
		else {
			LOG("passed 5050 buyt one of these dont work");
			LOG("loc.Y: {}, currentYBound: {}", loc.Y, currentYBound);
			LOG("yaw : {}, roll: {}, pitch: {}", yaw, roll, pitch);
		}
	}
	else if (loc.Y < -(currentYBound-10)) {
		if ( yaw1 && (roll2) ) {
			clampVal = 2;
			return Rotator{ pitch,1 ,expectedRoll2 };//16384
		}
		if ( yaw2 && (roll1) ) {
			LOG("clamped to blue back wall");
			clampVal = 2;
			return Rotator{ pitch,32768 ,expectedRoll1 };//south

		}

	}
	else if (loc.X > currentXBound-10) {
		if (yaw3 && (roll1))
		{
			LOG("clamped on the left wall");
			LOG("expected roll :{} ", expectedRoll1);
			clampVal = 3;
			//(pRoll1 || nRoll2)
			return Rotator{ pitch,south ,expectedRoll1 };
		}
		else if (yaw4 && (roll2))
		{
			LOG("clamped on the left wall");
			LOG("expected roll :{} ", expectedRoll2);
			clampVal = 3;

			return Rotator{ pitch,16384 ,expectedRoll2 };
		}
	}
	else if (loc.X < -(currentXBound-10))
	{
		if (yaw3 && (roll2))
		{
			LOG("clamped on the right wall");
			clampVal = 4;
			return Rotator{ pitch,south ,expectedRoll2 };
		}
		else if (yaw4 &&(roll1)) {
			LOG("clamped on the right wall1");
			clampVal = 4;
			return Rotator{ pitch,16384 ,expectedRoll1 };
		}
	}
	else if (loc.Z > 2000 && pitch1 && roll3) {
		LOG("clamped on the ceiling");
		clampVal = 5;
		if (pitch != 0) {
			LOG("pitch is changing");

		}
		if (roll != 32768) {
			LOG("roll is changing");
		}
		return Rotator{ 0,yaw,32768 };//0
	}

	//diagonal values?
	else if (cornerLine(loc.X, loc.Y) <= 0) {
		
		if (yaw7 && roll1) {
			LOG("clamped on the back right corner");
			clampVal = 6;
			return Rotator{ pitch,24576,south };
			
		}
		else if (yaw8 && roll2) {
			LOG("clamped on the back right corner");
			clampVal = 6;
			return Rotator{ pitch,57344,north };
		}
		
	}
	else if (cornerLine(loc.X, loc.Y) >= (diagBound - 25) * 2) {
		if (yaw7 && roll2) {
			LOG("clamped on the top left corner");
			clampVal = 7;
			return Rotator{ pitch,24576,north };
		}
		else if (yaw8 && roll1) {
			LOG("clamped on the top left corner");
			clampVal = 7;
			return Rotator{ pitch,57344,south };
		}
		
	}
	else if (cornerLine2(loc.X, loc.Y) <= 0) {
		
		if (yaw5 && roll1) {
			LOG("clamped on the top right corner");
			clampVal = 8;
			return Rotator{ pitch,8192,south };
		}else if (yaw6 && roll2) {
			LOG("clamped on the top right corner");
			clampVal = 8;
			return Rotator{ pitch,40960,north };
		}
	}
	else if (cornerLine2(loc.X, loc.Y) >= (diagBound - 25) * 2) {
		
		if (yaw5 && roll2) {
			LOG("clamped on the back left corner");
			clampVal = 9;
			return Rotator{ pitch,8192,north };
		}
		else if (yaw6 && roll1) {
			LOG("clamped on the back left corner");
			clampVal = 9;
			return Rotator{ pitch,40960,south };
		}
	}

		clampVal = 0;
		//LOG("yaw : {}, roll: {}, pitch: {}", yaw, roll,rot.Pitch);
		//LOG("X: {}, Y: {}, Z: {}", loc.X, loc.Y, loc.Z);
	return Rotator{ 0,0,0};

}


std::pair <float, float> VersatileTraining::getAxisBreakDown(Rotator rot,int extra) {
	if (isCeiling) {
		extra += 20;
	}

	//LOG("roll from getAxisBreakDown: {}", rot.Roll);
	// Determine how much of the extra should go to Z vs the axis
	auto getZWeight = [this](int roll) -> float {
		int center = 0;
		if (isCeiling) {
			center = (roll >= 0) ? 32768 : -32768;
		}
		float diff = std::abs(roll - center);
		float clamped = std::clamp(diff / 16384.0f, 0.0f, 1.0f);
		return 1.0f - clamped; // 1.0 to Z, 0.0 to other axis
		};

	float zWeight = getZWeight(rot.Roll);
	float zExtra = extra * zWeight;
	float axisExtra = extra * (1.0f - zWeight);

	//LOG("clampVal: {}", clampVal);
	//LOG("axisExtra: {}, zExtra: {}", axisExtra, zExtra);
	if (!isCeiling) {
		zExtra *= -1;
	}
	return std::make_pair(axisExtra, zExtra);

};

Vector VersatileTraining::getClampChange(Vector loc,Rotator rot) {

	auto perpendicularProjection = [](float m, float b, float a, float b0) -> std::pair<float, float> {
		float c = (a + m * (b0 - b)) / (m * m + 1);
		float d = m * c + b;
		return { c, d };
		};
	float cornerVal = diagBound + 80;
	
	checkForClamping(loc, rot);

	std::pair <float,float> axisBreakDown = getAxisBreakDown(rot,50);
	//LOG("axisBreakDown.first: {}, axisBreakDown.second: {}", axisBreakDown.first, axisBreakDown.second);
	
	//LOG("clampVal: {}", clampVal);
	//6 p->NewLocation.X + p->NewLocation.Y < -diagBound bottom right
	//7 diagBound < p->NewLocation.X + p->NewLocation.Y top left
	//8 (p->NewLocation.X - p->NewLocation.Y) > diagBound top right
	//9 (p->NewLocation.Y - p->NewLocation.X) > diagBound down left

	switch (clampVal) {//5090
	case 1: {
		LOG("Old location: {}, {} {}", loc.X, loc.Y,loc.Z);
		LOG("New location: {}, {} {}", loc.X, currentYBound + axisBreakDown.first, frozenZVal + axisBreakDown.second);
		return Vector{ loc.X,currentYBound + axisBreakDown.first,frozenZVal + axisBreakDown.second };
		break;
	}
	case 2: {
		return Vector{ loc.X,-(currentYBound + axisBreakDown.first),frozenZVal + axisBreakDown.second };

		break;
	}
	case 3: {
		
		return Vector{ (currentXBound+ axisBreakDown.first),loc.Y,frozenZVal + axisBreakDown.second };

		break;
	}
	case 4: {
		
		return Vector{ -(currentXBound+ axisBreakDown.first),loc.Y,frozenZVal + axisBreakDown.second };
		break;
	}
	case 5: {
		return Vector{ loc.X,loc.Y,2050 };
		break;
	}
		  //6 p->NewLocation.X + p->NewLocation.Y < -diagBound bottom right
	//7 diagBound < p->NewLocation.X + p->NewLocation.Y top left
	//8 (p->NewLocation.X - p->NewLocation.Y) > diagBound top right
	//9 (p->NewLocation.Y - p->NewLocation.X) > diagBound down left
	case 6: {
		//double m, double b, double a, double b0
		LOG("x : {}, y: {}, diagBound: {}", loc.X, loc.Y, cornerVal);
		std::pair<float, float> result = perpendicularProjection(-1, -cornerVal, loc.X, loc.Y);
		LOG("going to clamp on back right wall now");
		LOG("result.first: {}, result.second: {}, diagBound: {}", result.first, result.second, -cornerVal);
		return Vector{ result.first,result.second,loc.Z };
		break;
	}
	case 7: {
		
		std::pair<float, float> result = perpendicularProjection(-1, cornerVal, loc.X, loc.Y);
		LOG("going to clamp on front left wall now");
		LOG("result.first: {}, result.second: {}", result.first, result.second);
		return Vector{ result.first,result.second,loc.Z };
		break;
	}
	case 8: {
		std::pair<float, float> result = perpendicularProjection(1, cornerVal, loc.X, loc.Y);
		LOG("going to clamp on top right wall now");
		LOG("result.first: {}, result.second: {}", result.first, result.second);
		return Vector{ result.first,result.second,loc.Z };
		break;
	}
	case 9: {
		std::pair<float, float> result = perpendicularProjection(1, -cornerVal, loc.X, loc.Y);
		LOG("going to clamp on back left wall now");
		LOG("result.first: {}, result.second: {}", result.first, result.second);
		return Vector{ result.first,result.second,loc.Z };
		break;
	}
	default: {
		break;
	}
	}
	return Vector{ 0,0,0 };
}

Vector VersatileTraining::getStickingVelocity(Rotator rot) {
	float stickingAmount = 20;
	std::pair <float, float> axisBreakDown = getAxisBreakDown(rot, stickingAmount);
	switch (clampVal) {//5090
	case 1: {
		return Vector{ 0,axisBreakDown.first,axisBreakDown.second};
		break;
	}
	case 2: {
		return Vector{0,-axisBreakDown.first,axisBreakDown.second };

		break;
	}
	case 3: {
		return Vector{ axisBreakDown.first,0,axisBreakDown.second };

		break;
	}
	case 4: {
		return Vector{ -axisBreakDown.first,0,axisBreakDown.second };
		break;
	}
	case 5: {
		return Vector{ 0,0,stickingAmount };
		break;
	}
	case 6: {
		return  Vector{ -stickingAmount,-stickingAmount,0 };
		break;
	}
	case 7: {
		return Vector{ stickingAmount,stickingAmount,0 };
		break;
	}
	case 8: {
		return  Vector{ -stickingAmount,stickingAmount,0 };
		break;
	}
	case 9: {
		return Vector{ stickingAmount,-stickingAmount,0 };
		break;
	}
	default: {
		break;
	}
		}
	return Vector{ 0,0,0 };
}



void VersatileTraining::ApplyLocalPitch(float pitchInput) {
	// Get current rotation and position
	Rotator rot = currentRotation;
	Vector loc = currentLocation; // Get car location from your game wrapper

	// Only apply local pitch when on curves (0 < t < 1)
	if (t > 0.0f && t < 1.0f) {
		// Calculate blend factor based on how far we are into the curve
		float blendFactor = abs(t - 0.5f) * 2.0f; // 0 at middle, 1 at edges

		// Convert angles to radians
		const float UU_TO_RAD = 3.14159265358979323846f / 32768.0f;
		float rollRad = rot.Roll * UU_TO_RAD;

		// Compute local pitch adjustment
		float adjustedPitch = pitchInput * cos(rollRad) * blendFactor;
		float adjustedYaw = pitchInput * sin(rollRad) * blendFactor;

		// Blend with standard pitch
		rot.Pitch += static_cast<int>(adjustedPitch + pitchInput * (1.0f - blendFactor));
		rot.Yaw += static_cast<int>(adjustedYaw);
	}
	else {
		// Standard pitch behavior when not on curves
		rot.Pitch += static_cast<int>(pitchInput);
	}

	localRotation = rot;
}




void VersatileTraining::shiftToPositive(CustomTrainingData& data) {
	for (ShotState& shot : data.shots) {
		shot.startingVelocity += 2000;
		shot.goalBlocker.first.X += 910;
		shot.goalBlocker.second.X += 910;
		shot.goalBlocker.first.Z += 20;
		shot.goalBlocker.second.Z += 20;
	}
}
void VersatileTraining::shiftToNegative(CustomTrainingData& data) {
	for (ShotState& shot : data.shots) {
		shot.startingVelocity -= 2000;
		shot.goalBlocker.first.X -= 910;
		shot.goalBlocker.second.X -= 910;
		shot.goalBlocker.first.Z -= 20;
		shot.goalBlocker.second.Z -= 20;
	}

}