#include <pch.h>
#include "VersatileTraining.h"

int VersatileTraining::getRandomNumber(int min, int max) {
    std::random_device rd;  // Non-deterministic random number generator
    std::mt19937 gen(rd()); // Mersenne Twister engine
    std::uniform_int_distribution<> distrib(min, max);
    return distrib(gen);
}
