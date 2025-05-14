#include "pch.h"
#include "src/core/VersatileTraining.h"
#include "regex"


void VersatileTraining::SetDefaultKeybinds() {
    
    ClearAllKeybinds();

    cvarManager->setBind("F3", "open_gallery");
    cvarManager->setBind("X", "unlockCar");
    cvarManager->setBind("F", "freezeCar");
    cvarManager->setBind("J", "removeJump");
    cvarManager->setBind("B", "lockStartingVelocity");
    cvarManager->setBind("V", "lockScene");
    cvarManager->setBind("N", "startRecording");
    cvarManager->setBind("M", "spawnBot");
    cvarManager->setBind("G", "editGoalBlocker");
    cvarManager->setBind("K", "saveReplaySnapshot");
}

void VersatileTraining::ClearAllKeybinds() {
    
    for (const auto& [command, key] : currentBindings) {
        LOG("Removing bind: {} -> {}", key, command);
        cvarManager->removeBind(key);
    }

    currentBindings.clear();
    LOG("All plugin keybinds cleared");
}

void VersatileTraining::readCurrentBindings() {
    
    const char* appdata = std::getenv("APPDATA");
    if (appdata == nullptr) {
        LOG("Failed to get APPDATA environment variable");
        return;
    }
    std::string path = std::string(appdata) + "\\bakkesmod\\bakkesmod\\cfg\\binds.cfg";

    std::ifstream file(path);
    if (!file.is_open()) {
        LOG("Failed to open binds.cfg file: {}", path);
        return;
    }

    std::unordered_map<std::string, std::string> bindingsMap;

    
    std::vector<std::string> yourCommands = {
        "unlockCar", "freezeCar", "removeJump", "lockStartingVelocity",
        "saveReplaySnapshot", "lockScene",
        "startRecording", "spawnBot", "editGoalBlocker",
        "open_gallery", "printCurrentState", "dumpInputs"
    };

    
    std::regex pattern(R"(bind\s+(\S+)\s+\"([^\"]+)\")");
    std::string line;

    while (std::getline(file, line)) {
        std::smatch match;
        if (std::regex_search(line, match, pattern)) {
            std::string key = match[1].str();
            std::string command = match[2].str();

            
            for (const auto& cmd : yourCommands) {
                if (command == cmd || command.find(cmd) == 0) {
                    bindingsMap[command] = key;
                    break;
                }
            }
        }
    }

    currentBindings = bindingsMap;

    auto it = bindingsMap.find("open_gallery");
    if (it != bindingsMap.end()) {
        pastBinding = bind_key = it->second;
    }
    else {
        pastBinding = bind_key = "F3";
    }
}