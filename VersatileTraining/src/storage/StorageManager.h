#pragma once

#include "pch.h"








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

};
