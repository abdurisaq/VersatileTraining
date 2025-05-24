#pragma once
#include "pch.h"


struct PackInfo {
    std::string id;
    std::string name;
    std::string description;
    std::string code; // empty if null
    int difficulty;
    std::vector<std::string> tags;
    int totalShots;
    std::string packMetadataCompressed;
    std::string recordingDataCompressed;

    void print() const {
         
         
         
         
         
         
        for (const auto& tag : tags) {
			 
		}
         
		 
		 
	}
};


std::string extractValue(const std::string& str, const std::string& key);

std::vector<std::string> extractArray(const std::string& str, const std::string& key);

PackInfo parsePack(const std::string& json);


void packInfoToLocalStorage(const PackInfo packInfo, std::filesystem::path dataFolder, std::atomic<bool>& hasAction,
    std::string& pendingAction,
    std::mutex& pendingActionMutex );
void packInfoToLocalStorage(const PackInfo packInfo, std::filesystem::path dataFolder);