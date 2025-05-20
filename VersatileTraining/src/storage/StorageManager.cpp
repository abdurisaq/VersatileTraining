#include "pch.h"
#include "src/storage/StorageManager.h"


void StorageManager::saveCompressedTrainingDataWithRecordings(
    const std::unordered_map<std::string, CustomTrainingData>& trainingData,
    const std::filesystem::path& dataFolder) {

    

    std::filesystem::path trainingDataPath = dataFolder / "TrainingPacks";
    saveCompressedTrainingData(trainingData, trainingDataPath);
    CVarWrapper recordingEnabledCvar = _globalCvarManager->getCvar("versatile_recording_enabled");
    bool recordingFeaturesEnabled = recordingEnabledCvar ? recordingEnabledCvar.getBoolValue() : false;


    if (recordingFeaturesEnabled) {
        recordingStorage.saveAllRecordings(trainingData, trainingDataPath);
    }
}

std::unordered_map<std::string, CustomTrainingData> StorageManager::loadCompressedTrainingDataWithRecordings(
    const std::filesystem::path& dataFolder) {

    std::filesystem::path trainingDataPath = dataFolder / "TrainingPacks";
    auto trainingData = loadCompressedTrainingData(trainingDataPath);

    CVarWrapper recordingEnabledCvar = _globalCvarManager->getCvar("versatile_recording_enabled");
    bool recordingFeaturesEnabled = recordingEnabledCvar ? recordingEnabledCvar.getBoolValue() : false;


    if (recordingFeaturesEnabled) {
        recordingStorage.loadAllRecordings(trainingData, trainingDataPath);
    }
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


void StorageManager::savePackOverrideSettings(const std::map<std::string, PackOverrideSettings>& overrideSettings, const std::filesystem::path& filePath) {
    LOG("Saving pack override settings to binary file: {}", filePath.string());

    std::filesystem::path packOverRidePath = filePath / "packOverrideSettings.txt";

    std::ofstream outFile(packOverRidePath, std::ios::binary | std::ios::trunc);

    if (!outFile.is_open()) {
        LOG("Failed to open pack override settings file for writing: {}", packOverRidePath.string());
        return;
    }

    try {
        // Write the number of entries first
        size_t numEntries = overrideSettings.size();
        outFile.write(reinterpret_cast<const char*>(&numEntries), sizeof(numEntries));

        for (const auto& pair : overrideSettings) {
            // Write string key (pack code)
            size_t keyLength = pair.first.length();
            outFile.write(reinterpret_cast<const char*>(&keyLength), sizeof(keyLength));
            outFile.write(pair.first.c_str(), keyLength);

            // Write PackOverrideSettings struct
            outFile.write(reinterpret_cast<const char*>(&pair.second), sizeof(PackOverrideSettings));
        }
        outFile.close();
        LOG("Successfully saved {} pack override settings.", numEntries);
    }
    catch (const std::ios_base::failure& e) {
        LOG("Exception during saving pack override settings: {}", e.what());
        if (outFile.is_open()) outFile.close();
    }
}

std::map<std::string, PackOverrideSettings> StorageManager::loadPackOverrideSettings(const std::filesystem::path& filePath) {
    LOG("Loading pack override settings from binary file: {}", filePath.string());
    std::map<std::string, PackOverrideSettings> overrideSettings;

    std::filesystem::path packOverRidePath = filePath / "packOverrideSettings.txt";
    if (!std::filesystem::exists(packOverRidePath)) {
        LOG("Pack override settings file does not exist, returning empty map: {}", packOverRidePath.string());
        return overrideSettings;
    }

    std::ifstream inFile(packOverRidePath, std::ios::binary);
    if (!inFile.is_open()) {
        LOG("Failed to open pack override settings file for reading: {}", packOverRidePath.string());
        return overrideSettings;
    }

    try {
        // Read the number of entries
        size_t numEntries = 0;
        inFile.read(reinterpret_cast<char*>(&numEntries), sizeof(numEntries));
        if (inFile.gcount() != sizeof(numEntries)) {
            LOG("Failed to read number of entries or file is empty/corrupt.");
            inFile.close();
            return overrideSettings; // Or handle error differently
        }


        for (size_t i = 0; i < numEntries; ++i) {
            // Read string key (pack code)
            size_t keyLength = 0;
            inFile.read(reinterpret_cast<char*>(&keyLength), sizeof(keyLength));
            if (inFile.gcount() != sizeof(keyLength)) {
                LOG("Failed to read key length for entry {}.", i);
                break;
            }

            std::string key(keyLength, '\0');
            inFile.read(&key[0], keyLength);
            if (inFile.gcount() != keyLength) {
                LOG("Failed to read key data for entry {}.", i);
                break;
            }


            // Read PackOverrideSettings struct
            PackOverrideSettings settings;
            inFile.read(reinterpret_cast<char*>(&settings), sizeof(PackOverrideSettings));
            if (inFile.gcount() != sizeof(PackOverrideSettings)) {
                LOG("Failed to read settings data for entry {}.", i);
                break;
            }

            overrideSettings[key] = settings;
        }
        inFile.close();
        LOG("Successfully loaded {} pack override settings.", overrideSettings.size());
    }
    catch (const std::ios_base::failure& e) {
        LOG("Exception during loading pack override settings: {}. File might be corrupted.", e.what());
        if (inFile.is_open()) inFile.close();
        // Return whatever was successfully loaded, or an empty map
        return overrideSettings;
    }

    return overrideSettings;
}