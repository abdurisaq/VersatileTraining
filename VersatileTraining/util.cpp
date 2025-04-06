#include <pch.h>
#include "VersatileTraining.h"

int VersatileTraining::getRandomNumber(int min, int max) {
    std::random_device rd;  // Non-deterministic random number generator
    std::mt19937 gen(rd()); // Mersenne Twister engine
    std::uniform_int_distribution<> distrib(min, max);
    return distrib(gen);
}


Rotator VersatileTraining::checkForClamping(Vector loc, Rotator rot) {
	/*int yaw = abs(currentRotation.Yaw % 65536);*/
	int yaw = currentRotation.Yaw % 65536;
	/*int roll = abs(rot.Roll % 65536);*/
	int roll = (rot.Roll % 65536);
	//int roll = abs(currentRotation.Roll % 65536);

	bool yaw1 = ((yaw >= 0 && yaw <= 4000) || yaw >= 63000) || ((yaw <= 0 && yaw >= -4000) || yaw <= -63000);
	bool yaw2 = (yaw >= 30000 && yaw <= 35000)  || (yaw <= -30000 && yaw >= -35000);
	bool yaw3 = (yaw >= 40000 && yaw <= 50000) || ((yaw <= -15000 && yaw >= -18000));
	bool yaw4 = ((yaw >= 15000 && yaw <= 18000)) || (yaw <= -40000 && yaw >= -50000);
	/*bool nYaw1 = ((yaw <= 0 && yaw >= -4000) || yaw <= -63000);
	bool nYaw2 = (yaw <= -30000 && yaw >= -35000);
	bool nYaw3 = ((yaw <= -15000 && yaw >= -18000));
	bool nYaw4 = (yaw <= -40000 && yaw >= -50000);*/

	bool roll1 = (roll >= 40000 && roll <= 50000) || (roll <= -15000 && roll >= -17000);
	bool roll2 = (roll >= 15000 && roll <= 17000) || (roll >= -50000 && roll <= -40000);

	bool pRoll1 = (roll >= 40000 && roll <= 50000);
	bool pRoll2 = (roll >= 15000 && roll <= 17000);
	bool nRoll1 = (roll >= -50000 && roll <= -40000);
	bool nRoll2 = (roll <= -15000 && roll >= -17000);
	
	if (loc.Y > 5050) {

		if (yaw1  && (roll1))
		{

			LOG("clamped to orange back wall1");
			clampVal = 1;
			return Rotator{ 0,1, 49152 };
		}
		else if ( yaw2  && (roll2) )
		{
			LOG("clamped to orange back wall2");
			clampVal = 1;
			return Rotator{ 0,32768 ,16384 };
		}
		else {
			LOG("passed 5050 buyt one of these dont work");
		}
	}
	else if (loc.Y < -5050) {
		if ( yaw1 && (roll2) ) {
			clampVal = 2;
			return Rotator{ 0,1 ,16384 };
		}
		if ( yaw2 && (roll1) ) {
			LOG("clamped to blue back wall");
			clampVal = 2;
			return Rotator{ 0,32768 ,49152 };

		}

	}
	else if (loc.X > 4020) {
		if (yaw3 && (roll1))
		{
			LOG("clamped on the left wall");
			clampVal = 3;
			//(pRoll1 || nRoll2)
			return Rotator{ 0,49152 ,49152 };
		}
		else if (yaw4 && (roll2))
		{
			LOG("clamped on the left wall");
			clampVal = 3;

			return Rotator{ 0,16384 ,16384 };
		}
	}
	else if (loc.X < -4020)
	{
		if (yaw3 && (roll2))
		{
			LOG("clamped on the right wall");
			clampVal = 4;
			return Rotator{ 0,49152 ,16384 };
		}
		else if (yaw4 &&(roll1)) {
			LOG("clamped on the right wall1");
			clampVal = 4;
			return Rotator{ 0,16384 ,49152 };
		}
	}
	
	else {
		clampVal = 0;
		LOG("yaw : {}, roll: {}", yaw, roll);
		LOG("X: {}, Y: {}, Z: {}", loc.X, loc.Y, loc.Z);
	}
	return Rotator{ 0,0,0};

}


Vector VersatileTraining::getClampChange(Vector loc) {

	switch (clampVal) {//5090
	case 1: {
		return Vector{ loc.X,5090,loc.Z };
		break;
	}
	case 2: {
		return Vector{ loc.X,-5090,loc.Z };

		break;
	}
	case 3: {
		return Vector{ 4066,loc.Y,loc.Z };

		break;
	}
	case 4: {
		return Vector{ -4066,loc.Y,loc.Z };
		break;
	}
	default: {
		break;
	}
	}
	return Vector{ 0,0,0 };
}
