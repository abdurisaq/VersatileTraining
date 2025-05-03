#pragma once

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#define _AMD64_

// Windows APIs
#include <Windows.h>
#include <Xinput.h>
#pragma comment(lib, "XInput.lib")
#include <dinput.h>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

// Standard libraries
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <unordered_map>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <cmath>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <random>

// BakkesMod includes
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/wrappers/includes.h"

// External libraries - keep folder name casing consistent!
#include "external/RenderingTools/RenderingTools.h"
#include "external/IMGUI/imgui.h"
#include "external/IMGUI/imgui_stdlib.h"
#include "external/IMGUI/imgui_searchablecombo.h"
#include "external/IMGUI/imgui_rangeslider.h"

// Math constants
#define PI 3.14159265358979323846

// Logging
#include "include/logging.h"


#include "src/input/ControllerManager.h"
#include "src/input/ShotReplicationManager.h"
#include "src/utils/Math.h"
#include "src/training/CustomTrainingData.h"
#include "src/training/SnapShotManager.h"