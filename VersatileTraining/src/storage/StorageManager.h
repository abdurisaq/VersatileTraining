#pragma once

#include "pch.h"



struct SpecialKeybinds {
	int rollLeft = 'Q';    
	int rollRight = 'E';   
	int decreaseBoost = '1'; 
	int increaseBoost = '2'; 
	int decreaseVelocity = '3';
	int increaseVelocity = '4'; 
};




class StorageManager {

public:
	RecordingStorage recordingStorage;

	

	std::filesystem::path saveTrainingFilePath;
	std::filesystem::path saveReplayStateFilePath;


	


	void saveCompressedTrainingData(const std::unordered_map<std::string, CustomTrainingData>& trainingData, const std::filesystem::path& fileName);
	std::unordered_map<std::string, CustomTrainingData> loadCompressedTrainingData(const std::filesystem::path& fileName);


	std::vector<ReplayState> loadReplayStates(const std::filesystem::path& fileName);
	void saveReplayStates(const std::vector<ReplayState>& replays, const std::filesystem::path& fileName);


	void saveCompressedTrainingDataWithRecordings(const std::unordered_map<std::string, CustomTrainingData>& trainingData,
		const std::filesystem::path& dataFolder);

	std::unordered_map<std::string, CustomTrainingData> loadCompressedTrainingDataWithRecordings(
		const std::filesystem::path& dataFolder);


	void saveSpecialKeybinds(SpecialKeybinds keybinds, const std::filesystem::path& dataFolder);
	SpecialKeybinds loadSpecialKeybinds(const std::filesystem::path& dataFolder);


};
