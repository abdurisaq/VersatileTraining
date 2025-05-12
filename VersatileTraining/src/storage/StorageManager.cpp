#include "pch.h"
#include "src/storage/StorageManager.h"


void StorageManager::saveCompressedTrainingDataWithRecordings(
    const std::unordered_map<std::string, CustomTrainingData>& trainingData,
    const std::filesystem::path& dataFolder) {

    

    std::filesystem::path trainingDataPath = dataFolder / "TrainingPacks";
    saveCompressedTrainingData(trainingData, trainingDataPath);

    recordingStorage.saveAllRecordings(trainingData, trainingDataPath);
}

std::unordered_map<std::string, CustomTrainingData> StorageManager::loadCompressedTrainingDataWithRecordings(
    const std::filesystem::path& dataFolder) {

    std::filesystem::path trainingDataPath = dataFolder / "TrainingPacks";
    auto trainingData = loadCompressedTrainingData(trainingDataPath);

    recordingStorage.loadAllRecordings(trainingData, trainingDataPath);

    return trainingData;
}