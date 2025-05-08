#include "pch.h"
#include "src/storage/StorageManager.h"


void StorageManager::saveCompressedTrainingDataWithRecordings(
    const std::unordered_map<std::string, CustomTrainingData>& trainingData,
    const std::filesystem::path& dataFolder) {

    

    std::filesystem::path trainingDataPath = dataFolder / "packs.txt";
    saveCompressedTrainingData(trainingData, trainingDataPath);

    recordingStorage.saveAllRecordings(trainingData, dataFolder);
}

std::unordered_map<std::string, CustomTrainingData> StorageManager::loadCompressedTrainingDataWithRecordings(
    const std::filesystem::path& dataFolder) {

    std::filesystem::path trainingDataPath = dataFolder / "packs.txt";
    auto trainingData = loadCompressedTrainingData(trainingDataPath);

    recordingStorage.loadAllRecordings(trainingData, dataFolder);

    return trainingData;
}