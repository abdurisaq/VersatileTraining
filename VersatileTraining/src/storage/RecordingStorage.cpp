#include "pch.h"

#include "RecordingStorage.h"


void RecordingStorage::saveAllRecordings(const std::unordered_map<std::string, CustomTrainingData>& trainingData,
    const std::filesystem::path& basePath) {
    


   
    std::filesystem::create_directories(basePath);

    // Process each training pack
    for (const auto & [packId, data] : trainingData) {
        if (packId.empty()) continue;
        std::string sanitizedId;
        if (data.code.empty()) {
            sanitizedId = sanitizeFilename(packId);
        }
        else {
			sanitizedId = data.code;
		}
        LOG("Num Shots: {}", data.numShots);
        /*for (int i = 0; i < data.numShots; i++) {
            LOG("Shot {}: Boost Amount: {}, Starting Velocity: {}, Freeze Car: {}, has jump: {}", i, data.shots[i].boostAmount, data.shots[i].startingVelocity, static_cast<int>(data.shots[i].freezeCar), static_cast<int>(data.shots[i].hasJump));
            LOG("has recording? {}", data.shots[i].recording.inputs.size() > 0 ? "true" : "false");
        }*/
        std::filesystem::path packFolder = basePath / sanitizedId;
        std::filesystem::create_directories(packFolder);

        savePackRecordings(packId, data, packFolder);
    }

    LOG("All shot recordings saved successflly.");
}

void RecordingStorage::savePackRecordings(const std::string& packId, const CustomTrainingData& data,
    const std::filesystem::path& packFolder) {
    // metadata
    std::ofstream metaFile(packFolder / "meta.json");
    metaFile << "{\"id\":\"" << packId << "\",\"name\":\"" << data.name
        << "\",\"shots\":" << data.shots.size() << "}\n";
    metaFile.close();

    
    std::filesystem::path recordingsFile = packFolder / "shots.rec";
    std::ofstream outFile(recordingsFile);

    if (!outFile) {
        LOG("Failed to create recordings file for pack {}", packId);
        return;
    }

    // start's file with numshots
    outFile << data.shots.size() << "\n";

    
    int validShotCount = 0;
    for (size_t i = 0; i < data.shots.size(); i++) {
        const ShotRecording& recording = data.shots[i].recording;

        
        if (recording.carBody == 0) {
            LOG("Recording is empty, skipping shot {} in pack {}", i, packId);
            
            outFile << "EMPTY\n";
            continue;
        }

        std::vector<uint8_t> compressed = compressShotRecording(recording);
        LOG("Compressed data size for shot {}: {} bytes", i, compressed.size());

        char* base64Out = new char[compressed.size() * 2]; // Ensure enough space (4/3 ratio)
        size_t base64OutLen = 0;
        base64_encode((const char*)compressed.data(), compressed.size(), base64Out, &base64OutLen, 0);
        std::string base64Data(base64Out, base64OutLen);
        delete[] base64Out;
     
        outFile << base64Data << "\n";
        validShotCount++;
    }

    outFile.close();
    LOG("Saved {} valid recordings for pack {}", validShotCount, packId);
}

void RecordingStorage::loadAllRecordings(std::unordered_map<std::string, CustomTrainingData>& trainingData,
    const std::filesystem::path& basePath) {
    

    if (!std::filesystem::exists(basePath)) {
        LOG("No recordings directory found at {}", basePath.string());
        return;
    }

    //diff folders
    for (const auto& entry : std::filesystem::directory_iterator(basePath)) {
        if (!entry.is_directory()) continue;

        std::string packId = entry.path().filename().string();

        LOG("entry.path : {}", entry.path().string());

        
        if (trainingData.find(packId) != trainingData.end()) {
            loadPackRecordings(packId, trainingData[packId], entry.path());
        }
    }

    LOG("All shot recordings loaded successfully.");
}

void RecordingStorage::loadPackRecordings(const std::string& packId, CustomTrainingData& data,
    const std::filesystem::path& packFolder) {

    std::filesystem::path recordingsFile = packFolder / "shots.rec";

    if (!std::filesystem::exists(recordingsFile)) {
        LOG("No recordings file found for pack {}", packId);
        return;
    }

    std::ifstream inFile(recordingsFile);
    if (!inFile) {
        LOG("Failed to open recordings file for pack {}", packId);
        return;
    }

    
    std::string countLine;
    std::getline(inFile, countLine);
    int totalShots = std::stoi(countLine);

    if (data.shots.size() < totalShots) {
        LOG("Expanding shots array from {} to {}", data.shots.size(), totalShots);
        data.shots.resize(totalShots);
    }

    // Read each shot
    std::string line;
    size_t shotIndex = 0;

    while (std::getline(inFile, line) && shotIndex < totalShots) {
        if (line == "EMPTY") {
            LOG("Shot {} marked as empty", shotIndex);
            shotIndex++;
            continue;
        }

 

        char* decodedOut = new char[line.size()];
        size_t decodedOutLen = 0;
        base64_decode(line.c_str(), line.size(), decodedOut, &decodedOutLen, 0);
        std::string output(decodedOut, decodedOutLen);
        delete[] decodedOut;

        std::vector<uint8_t> compressed(output.begin(), output.end() - 1);

        if (!compressed.empty()) {
            ShotRecording recording = decompressShotRecording(compressed);
            data.shots[shotIndex].recording = recording;

            LOG("Loaded recording for pack {} shot {} with {} inputs",
                packId, shotIndex, recording.inputs.size());
        }

        shotIndex++;
    }

    inFile.close();
}
std::vector<uint8_t> RecordingStorage::compressShotRecording(const ShotRecording& recording) {
    // header | carbody | input count | gamepad settings | location | rotation | velocity | compressed data
    std::vector<uint8_t> header;
    uint32_t carBody = recording.carBody;
    uint32_t inputCount = recording.inputs.size();

    // Use default car body if unset
    if (carBody == 0) carBody = 23;

    // Add car body to header (4 bytes)
    header.push_back((carBody >> 24) & 0xFF);
    header.push_back((carBody >> 16) & 0xFF);
    header.push_back((carBody >> 8) & 0xFF);
    header.push_back(carBody & 0xFF);

    // Add input count to header (4 bytes)
    header.push_back((inputCount >> 24) & 0xFF);
    header.push_back((inputCount >> 16) & 0xFF);
    header.push_back((inputCount >> 8) & 0xFF);
    header.push_back(inputCount & 0xFF);

    float settings[4] = {
        recording.settings.ControllerDeadzone,
        recording.settings.DodgeInputThreshold,
        recording.settings.SteeringSensitivity,
        recording.settings.AirControlSensitivity
    };

    for (int i = 0; i < 4; i++) {
        uint32_t bits = *reinterpret_cast<uint32_t*>(&settings[i]);
        header.push_back((bits >> 24) & 0xFF);
        header.push_back((bits >> 16) & 0xFF);
        header.push_back((bits >> 8) & 0xFF);
        header.push_back(bits & 0xFF);
    }

    
    Vector loc = recording.initialState.location;
    for (float val : {loc.X, loc.Y, loc.Z}) {
        uint32_t bits = *reinterpret_cast<uint32_t*>(&val);
        header.push_back((bits >> 24) & 0xFF);
        header.push_back((bits >> 16) & 0xFF);
        header.push_back((bits >> 8) & 0xFF);
        header.push_back(bits & 0xFF);
    }

   
    Rotator rot = recording.initialState.rotation;
    for (float val : {rot.Pitch, rot.Yaw, rot.Roll}) {
        uint32_t bits = *reinterpret_cast<uint32_t*>(&val);
        header.push_back((bits >> 24) & 0xFF);
        header.push_back((bits >> 16) & 0xFF);
        header.push_back((bits >> 8) & 0xFF);
        header.push_back(bits & 0xFF);
    }

    
    Vector vel = recording.initialState.velocity;
    for (float val : {vel.X, vel.Y, vel.Z}) {
        uint32_t bits = *reinterpret_cast<uint32_t*>(&val);
        header.push_back((bits >> 24) & 0xFF);
        header.push_back((bits >> 16) & 0xFF);
        header.push_back((bits >> 8) & 0xFF);
        header.push_back(bits & 0xFF);
    }

   
    std::vector<ShotRecording> singleRecording = { recording };

    std::vector<uint8_t> encodedData = encode_recordings(singleRecording);

    std::vector<uint8_t> compressedData = compress_data(encodedData);

    std::vector<uint8_t> result = header;
    result.insert(result.end(), compressedData.begin(), compressedData.end());

    return result;
}

ShotRecording RecordingStorage::decompressShotRecording(const std::vector<uint8_t>& compressedData) {
    ShotRecording recording;

    // gotta be bigger than what a header is atleast
    if (compressedData.size() < 60) {
        LOG("Compressed data too small to contain a valid header (size: {})", compressedData.size());
        return recording;
    }
    recording.carBody = (compressedData[0] << 24) |
        (compressedData[1] << 16) |
        (compressedData[2] << 8) |
        compressedData[3];

    
    uint32_t inputCount = (compressedData[4] << 24) |
        (compressedData[5] << 16) |
        (compressedData[6] << 8) |
        compressedData[7];

    
    uint32_t dz_bits = (compressedData[8] << 24) |
        (compressedData[9] << 16) |
        (compressedData[10] << 8) |
        compressedData[11];

    uint32_t ddz_bits = (compressedData[12] << 24) |
        (compressedData[13] << 16) |
        (compressedData[14] << 8) |
        compressedData[15];

    uint32_t ss_bits = (compressedData[16] << 24) |
        (compressedData[17] << 16) |
        (compressedData[18] << 8) |
        compressedData[19];

    uint32_t as_bits = (compressedData[20] << 24) |
        (compressedData[21] << 16) |
        (compressedData[22] << 8) |
        compressedData[23];

    float deadzone = *reinterpret_cast<float*>(&dz_bits);
    float dodgeDeadzone = *reinterpret_cast<float*>(&ddz_bits);
    float steeringSensitivity = *reinterpret_cast<float*>(&ss_bits);
    float aerialSensitivity = *reinterpret_cast<float*>(&as_bits);

    recording.settings = GamepadSettings(
        deadzone, dodgeDeadzone, steeringSensitivity, aerialSensitivity);

    uint32_t locX = (compressedData[24] << 24) | (compressedData[25] << 16) |
        (compressedData[26] << 8) | compressedData[27];
    uint32_t locY = (compressedData[28] << 24) | (compressedData[29] << 16) |
        (compressedData[30] << 8) | compressedData[31];
    uint32_t locZ = (compressedData[32] << 24) | (compressedData[33] << 16) |
        (compressedData[34] << 8) | compressedData[35];

    recording.initialState.location.X = *reinterpret_cast<float*>(&locX);
    recording.initialState.location.Y = *reinterpret_cast<float*>(&locY);
    recording.initialState.location.Z = *reinterpret_cast<float*>(&locZ);

    
    uint32_t rotP = (compressedData[36] << 24) | (compressedData[37] << 16) |
        (compressedData[38] << 8) | compressedData[39];
    uint32_t rotY = (compressedData[40] << 24) | (compressedData[41] << 16) |
        (compressedData[42] << 8) | compressedData[43];
    uint32_t rotR = (compressedData[44] << 24) | (compressedData[45] << 16) |
        (compressedData[46] << 8) | compressedData[47];

    recording.initialState.rotation.Pitch = *reinterpret_cast<float*>(&rotP);
    recording.initialState.rotation.Yaw = *reinterpret_cast<float*>(&rotY);
    recording.initialState.rotation.Roll = *reinterpret_cast<float*>(&rotR);

    
    uint32_t velX = (compressedData[48] << 24) | (compressedData[49] << 16) |
        (compressedData[50] << 8) | compressedData[51];
    uint32_t velY = (compressedData[52] << 24) | (compressedData[53] << 16) |
        (compressedData[54] << 8) | compressedData[55];
    uint32_t velZ = (compressedData[56] << 24) | (compressedData[57] << 16) |
        (compressedData[58] << 8) | compressedData[59];

    recording.initialState.velocity.X = *reinterpret_cast<float*>(&velX);
    recording.initialState.velocity.Y = *reinterpret_cast<float*>(&velY);
    recording.initialState.velocity.Z = *reinterpret_cast<float*>(&velZ);

    
    std::vector<uint8_t> justCompressedData(
        compressedData.begin() + 60, compressedData.end());

    std::vector<uint8_t> decompressedData = decompress_data(justCompressedData);

    
    std::vector<ShotRecording> recordings = decode_recordings(decompressedData, inputCount);

    if (!recordings.empty()) {
        recording.inputs = recordings[0].inputs;
    }

    return recording;
}

std::filesystem::path RecordingStorage::getRecordingPath(const std::filesystem::path& packFolder, int shotIndex) {
    std::filesystem::path path = packFolder / (std::to_string(shotIndex) + ".rec");
    std::string pathStr = path.string();
    LOG("saving at path: {}", pathStr);  // debug pathing
    return packFolder / (std::to_string(shotIndex) + ".rec");
}

// just incase rl training pack name is weird, and will mess with folder name
std::string RecordingStorage::sanitizeFilename(const std::string& input) {
    std::string result = input;
    const std::string illegalChars = "\\/:*?\"<>|";
    for (auto c : illegalChars) {
        result.erase(std::remove(result.begin(), result.end(), c), result.end());
    }
    return result;
}
