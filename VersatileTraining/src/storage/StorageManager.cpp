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




void StorageManager::saveSpecialKeybinds(SpecialKeybinds keybinds, const std::filesystem::path& dataFolder) {
    std::filesystem::path specialBindsPath = dataFolder / "specialbinds.cfg";
    std::ofstream file(specialBindsPath);

    if (file.is_open()) {
        file << keybinds.rollLeft << std::endl;
        file << keybinds.rollRight << std::endl;
        file << keybinds.decreaseBoost << std::endl;
        file << keybinds.increaseBoost << std::endl;
        file << keybinds.decreaseVelocity << std::endl;
        file << keybinds.increaseVelocity << std::endl;
        LOG("Special keybinds saved to: {}", specialBindsPath.string());
    }
    else {
        LOG("Failed to save special keybinds to: {}", specialBindsPath.string());
    }

}
SpecialKeybinds StorageManager::loadSpecialKeybinds(const std::filesystem::path& dataFolder) {
    std::filesystem::path specialBindsPath = dataFolder / "specialbinds.cfg";

    // Set defaults in case file doesn't exist or can't be read
    SpecialKeybinds specialKeybinds = {
        'Q', 'E', '1', '2', '3', '4' 
    };
    std::ifstream file(specialBindsPath);
    if (file.is_open()) {
        std::string line;
        if (std::getline(file, line)) specialKeybinds.rollLeft = std::stoi(line);
        if (std::getline(file, line)) specialKeybinds.rollRight = std::stoi(line);
        if (std::getline(file, line)) specialKeybinds.decreaseBoost = std::stoi(line);
        if (std::getline(file, line)) specialKeybinds.increaseBoost = std::stoi(line);
        if (std::getline(file, line)) specialKeybinds.decreaseVelocity = std::stoi(line);
        if (std::getline(file, line)) specialKeybinds.increaseVelocity = std::stoi(line);
        LOG("Special keybinds loaded from: {}", specialBindsPath.string());
    }
    else {
        LOG("No special keybinds file found, using defaults");
    }

    return specialKeybinds;


}