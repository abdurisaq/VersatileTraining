#include "pch.h"

#include "src/storage/StorageManager.h"


constexpr size_t NAME_LEN_BITS = 5;
constexpr size_t NUM_SHOTS_BITS = 6;
constexpr size_t BOOST_MIN_BITS = 7;
constexpr size_t VELOCITY_MIN_BITS = 13;
constexpr size_t GOAL_MIN_BITS = 12;
constexpr size_t NUM_FLOAT_BITS = 16;

constexpr size_t TRAINING_CODE_FLAG_BITS = 1;
constexpr size_t TRAINING_CODE_CHARS = 19;
inline static size_t CalculateRequiredBits(int range) {
    return (size_t)std::ceil(std::log2(range));
}


uint8_t CompressVectors(
    const std::vector<Vector>& vectors,
    float maxMagnitude,
    std::vector<uint8_t>& bitstream,
    size_t& bitIndexInByte,
    uint8_t currentByte,
    const std::string& debugLabel = ""
) {
    const int numBits = 16; // haven't changed this yet, works for now
    const float range = 2.0f * maxMagnitude;



    for (const Vector& v : vectors) {
        for (float value : {v.X, v.Y, v.Z}) {
            float clamped = std::clamp(value, -maxMagnitude, maxMagnitude);
            float normalized = (clamped + maxMagnitude) / range;
            uint32_t quantized = static_cast<uint32_t>(std::round(normalized * ((1 << numBits) - 1)));
            for (int i = 0; i < numBits; ++i) {
                bool bit = (quantized >> (numBits - 1 - i)) & 1;
                currentByte |= (bit << (7 - bitIndexInByte));
                bitIndexInByte++;

                if (bitIndexInByte == 8) {
                    bitstream.push_back(currentByte);
                    currentByte = 0;
                    bitIndexInByte = 0;
                }
            }
        }
    }

    return currentByte;
}

uint8_t CompressIntegers(
    const std::vector<int>& values,
    int globalMin,
    int globalMax,
    std::vector<uint8_t>& bitstream,
    size_t& bitIndexInByte,
    uint8_t currentByte,
    const std::string& debugLabel = ""
) {
    int range = globalMax - globalMin;
    size_t numBits = CalculateRequiredBits(range);


    for (int value : values) {
        int compressed = value - globalMin;

        for (size_t i = 0; i < numBits; ++i) {
            bool bit = (compressed >> (numBits - 1 - i)) & 1;
            currentByte |= (bit << (7 - bitIndexInByte));
            bitIndexInByte++;

            if (bitIndexInByte == 8) {
                bitstream.push_back(currentByte);
                currentByte = 0;
                bitIndexInByte = 0;
            }
        }
    }

    return currentByte;
}

uint8_t CompressBits(const std::vector<bool>& freezeCar, std::vector<uint8_t>& bitstream, size_t& bitCount, uint8_t currentByte, bool finalWrite) {


    for (bool freeze : freezeCar) {
        if (freeze) {
            currentByte |= (1 << (7 - bitCount));
        }
        bitCount++;

        //dont change
        if (bitCount == 8) {
            bitstream.push_back(currentByte);
            currentByte = 0;
            bitCount = 0;
        }
    }
    if (finalWrite && bitCount > 0) {
        bitstream.push_back(currentByte);
        currentByte = 0;
        bitCount = 0;
    }
    return currentByte;
}

size_t ReadBits(const std::vector<uint8_t>& bitstream, size_t& bitIndex, size_t numBits) {
    size_t value = 0;
    size_t byteIndex = bitIndex / 8;
    size_t bitOffset = bitIndex % 8;

    for (size_t i = 0; i < numBits; ++i) {
        // 
        if (bitstream[byteIndex] & (1 << (7 - bitOffset))) {
            value |= (1 << (numBits - 1 - i));
        }


        if (++bitOffset == 8) {
            bitOffset = 0;
            ++byteIndex;
        }
    }

    bitIndex += numBits;
    return value;
}

void DecompressVectors(
    std::vector<Vector>& output,
    float maxMagnitude,
    const std::vector<uint8_t>& bitstream,
    size_t& bitIndex,
    const std::string& debugLabel = ""
) {

    int numBits = 16;
    const float range = 2.0f * maxMagnitude;
    const float scale = 1.0f / ((1 << numBits) - 1);


    for (size_t i = 0; i < output.size(); ++i) {
        uint32_t xBits = ReadBits(bitstream, bitIndex, numBits);
        uint32_t yBits = ReadBits(bitstream, bitIndex, numBits);
        uint32_t zBits = ReadBits(bitstream, bitIndex, numBits);

        output[i].X = (xBits * scale * range) - maxMagnitude;
        output[i].Y = (yBits * scale * range) - maxMagnitude;
        output[i].Z = (zBits * scale * range) - maxMagnitude;
    }
}

void DecompressIntegers(
    std::vector<int>& output,
    size_t globalMin,
    int numBits,
    const std::vector<uint8_t>& bitstream,
    size_t& bitIndex,
    const std::string& debugLabel = ""
) {

    for (size_t i = 0; i < output.size(); ++i) {
        size_t compressed = ReadBits(bitstream, bitIndex, numBits);
        output[i] = (int)(compressed + globalMin);
    }
}


void DecompressBits(std::vector<bool>& vec, std::vector<uint8_t>& bitstream, size_t& bitIndex) {
    size_t bitCount = 0;
    for (size_t i = 0; i < vec.size(); ++i) {
        size_t bit = ReadBits(bitstream, bitIndex, 1);
        vec[i] = (bit != 0);
    }
}
std::pair<int, int> getMinMaxAmount(std::vector<int> arr) {
    int min = arr[0];
    int max = arr[0];
    for (int i = 1; i < arr.size(); i++) {
        if (arr[i] < min) {
            min = arr[i];
        }
        if (arr[i] > max) {
            max = arr[i];
        }
    }
    return { min, max };
}

uint8_t writeBits(uint8_t byte, size_t& bitIndex, std::vector<uint8_t>& bitstream, const std::vector<uint8_t>& data, int numBitsToWrite) {


    bool lastByte = false;
    for (int i = 0; i < numBitsToWrite; i++) {

        if (!lastByte && (i == 0 || i % 8 == 0) && (numBitsToWrite - i <= 8)) {
            lastByte = true;
        }
        bool bit = (data[i / 8] >> (7 - (i % 8))) & 1;
        if (lastByte) {
            bit = (data[i / 8] & (1 << (numBitsToWrite - i - 1)));
        }
        byte |= (bit << (7 - bitIndex));

        bitIndex++;


        if (bitIndex == 8) {
            bitstream.push_back(byte);
            byte = 0;
            bitIndex = 0;
        }

    }
    return byte;

}

void StorageManager::saveCompressedTrainingData(const std::unordered_map<std::string, CustomTrainingData>& trainingData, const std::filesystem::path& trainingPacksFolder) {
    std::filesystem::create_directories(trainingPacksFolder);

    for (std::pair<std::string, CustomTrainingData> entry : trainingData) {
        
        const CustomTrainingDataflattened& data = entry.second.deflate();

        std::string folderName = !data.code.empty() ? data.code : data.name;


        for (char& c : folderName) {
            if (c == '<' || c == '>' || c == ':' || c == '"' || c == '/' ||
                c == '\\' || c == '|' || c == '?' || c == '*') {
                c = '_';
            }
        }

        std::filesystem::path packFolder = trainingPacksFolder / folderName;
        std::filesystem::create_directories(packFolder);

        std::filesystem::path packFile = packFolder / "trainingpack.txt";

        std::ofstream outFile(packFile, std::ios::binary);
        if (!outFile.is_open()) {

            continue;
        }




        // Header: nameLength  | numShots | minBoost | minVelocity | boostbitAmount | velocitybitAmount | name
        size_t nameLength = data.name.size();
        std::vector<uint8_t> bitstream;
        uint8_t byte = 0;
        size_t bitIndexInByte = 0;
        std::vector<uint8_t> binaryDataToWrite;

        bool hasCode = !data.code.empty() && data.code.length() == TRAINING_CODE_CHARS;
        binaryDataToWrite.push_back(hasCode ? 1 : 0);

        byte = writeBits(byte, bitIndexInByte, bitstream, binaryDataToWrite, TRAINING_CODE_FLAG_BITS);

        if (hasCode) {
            for (char c : data.code) {
                binaryDataToWrite.clear();
                binaryDataToWrite.push_back(static_cast<uint8_t>(c));
                byte = writeBits(byte, bitIndexInByte, bitstream, binaryDataToWrite, 8);
            }

        }


        binaryDataToWrite.clear();
        binaryDataToWrite.push_back(nameLength & 0xFF);
        byte = writeBits(byte, bitIndexInByte, bitstream, binaryDataToWrite, NAME_LEN_BITS);


        binaryDataToWrite.clear();
        binaryDataToWrite.push_back(data.numShots & 0xFF);
        byte = writeBits(byte, bitIndexInByte, bitstream, binaryDataToWrite, NUM_SHOTS_BITS);




        std::pair<int, int> boundaryBoosts = getMinMaxAmount(data.boostAmounts);
        std::pair<int, int> boundaryVelocities = getMinMaxAmount(data.startingVelocity);


        binaryDataToWrite.clear();
        binaryDataToWrite.push_back(boundaryBoosts.first & 0xFF);
        byte = writeBits(byte, bitIndexInByte, bitstream, binaryDataToWrite, BOOST_MIN_BITS);

        binaryDataToWrite.clear();
        binaryDataToWrite.push_back((boundaryVelocities.first >> 5) & 0xFF);
        binaryDataToWrite.push_back((boundaryVelocities.first) & 0x1F);
        byte = writeBits(byte, bitIndexInByte, bitstream, binaryDataToWrite, VELOCITY_MIN_BITS);


        std::vector<int> magnitudes;
        for (const auto& vec : data.extendedStartingVelocities) {
            int magnitude = static_cast<int>(vec.magnitude());
            magnitudes.push_back(magnitude);
        }
        std::pair<int, int> magnitudeBounds = getMinMaxAmount(magnitudes);

        binaryDataToWrite.clear();
        binaryDataToWrite.push_back((magnitudeBounds.second >> 5) & 0xFF);
        binaryDataToWrite.push_back((magnitudeBounds.second & 0x1F));

        byte = writeBits(byte, bitIndexInByte, bitstream, binaryDataToWrite, VELOCITY_MIN_BITS);

        std::vector<int> magnitudesAng;
        for (const auto& vec : data.extendedStartingAngularVelocities) {
            int magnitude = static_cast<int>(vec.magnitude());
            magnitudesAng.push_back(magnitude);
        }
        std::pair<int, int> magnitudeBoundsAng = getMinMaxAmount(magnitudesAng);
        magnitudeBoundsAng.first++;
        magnitudeBoundsAng.second++;

        binaryDataToWrite.clear();
        binaryDataToWrite.push_back((magnitudeBoundsAng.second) & 0xFF);
        byte = writeBits(byte, bitIndexInByte, bitstream, binaryDataToWrite, 8);
        std::vector<int> xVals;
        std::vector<int> zVals;

        xVals.reserve(data.goalBlockers.size() * 2);
        zVals.reserve(data.goalBlockers.size() * 2);

        for (const auto& [first, second] : data.goalBlockers) {
            xVals.push_back(static_cast<int>(first.X));
            xVals.push_back(static_cast<int>(second.X));
            zVals.push_back(static_cast<int>(first.Z));
            zVals.push_back(static_cast<int>(second.Z));
        }
        std::pair<int, int> boundaryX = getMinMaxAmount(xVals);


        binaryDataToWrite.clear();
        binaryDataToWrite.push_back((boundaryX.first >> 4) & 0xFF);
        binaryDataToWrite.push_back((boundaryX.first) & 0xF);
        byte = writeBits(byte, bitIndexInByte, bitstream, binaryDataToWrite, GOAL_MIN_BITS);

        std::pair<int, int> boundaryZ = getMinMaxAmount(zVals);

        binaryDataToWrite.clear();
        binaryDataToWrite.push_back((boundaryZ.first >> 4) & 0xFF);
        binaryDataToWrite.push_back((boundaryZ.first) & 0xF);
        byte = writeBits(byte, bitIndexInByte, bitstream, binaryDataToWrite, GOAL_MIN_BITS);


        size_t numBitsForBoost = 0;
        if (boundaryBoosts.first != boundaryBoosts.second) {
            numBitsForBoost = CalculateRequiredBits(boundaryBoosts.second - boundaryBoosts.first);
        }


        size_t numBitsForVelocity = 0;
        if (boundaryVelocities.first != boundaryVelocities.second) {
            numBitsForVelocity = CalculateRequiredBits(boundaryVelocities.second - boundaryVelocities.first);
        }





        size_t numBitsForXBlocker = 0;
        if (boundaryX.first != boundaryX.second) {
            numBitsForXBlocker = CalculateRequiredBits(boundaryX.second - boundaryX.first);
        }
        size_t numBitsForZBlocker = 0;
        if (boundaryZ.first != boundaryZ.second) {
            numBitsForZBlocker = CalculateRequiredBits(boundaryZ.second - boundaryZ.first);
        }

        binaryDataToWrite.clear();
        binaryDataToWrite.push_back(numBitsForBoost & 0xFF);
        byte = writeBits(byte, bitIndexInByte, bitstream, binaryDataToWrite, 3);

        binaryDataToWrite.clear();
        binaryDataToWrite.push_back(numBitsForVelocity & 0xFF);

        byte = writeBits(byte, bitIndexInByte, bitstream, binaryDataToWrite, 4);






        binaryDataToWrite.clear();
        binaryDataToWrite.push_back(numBitsForXBlocker & 0xFF);
        byte = writeBits(byte, bitIndexInByte, bitstream, binaryDataToWrite, 4);

        binaryDataToWrite.clear();
        binaryDataToWrite.push_back(numBitsForZBlocker & 0xFF);
        byte = writeBits(byte, bitIndexInByte, bitstream, binaryDataToWrite, 4);


        for (char c : data.name) {
            std::vector<uint8_t> charBits = { static_cast<uint8_t>(c) };
            byte = writeBits(byte, bitIndexInByte, bitstream, charBits, 7);
        }

        // Compress 
        if (numBitsForBoost > 0) {
            byte = CompressIntegers(data.boostAmounts, boundaryBoosts.first, boundaryBoosts.second, bitstream, bitIndexInByte, byte, "boosts");
        }
        if (numBitsForVelocity > 0) {
            byte = CompressIntegers(data.startingVelocity, boundaryVelocities.first, boundaryVelocities.second, bitstream, bitIndexInByte, byte, "velocity");
        }
        byte = CompressVectors(data.extendedStartingVelocities, magnitudeBounds.second, bitstream, bitIndexInByte, byte, "extended starting velocity");

        std::vector<bool> hasAngularVelocity;
        std::vector<Vector> angularVelocitiesToSave;
        for (const auto& vec : data.extendedStartingAngularVelocities) {
            if (vec.X != 0 || vec.Y != 0 || vec.Z != 0) {
                hasAngularVelocity.push_back(true);
                angularVelocitiesToSave.push_back(vec);
            }
            else {
                hasAngularVelocity.push_back(false);
            }
        }

        byte = CompressBits(hasAngularVelocity, bitstream, bitIndexInByte, byte, false);
        byte = CompressVectors(angularVelocitiesToSave, magnitudeBoundsAng.second, bitstream, bitIndexInByte, byte, "extended starting angular velocity");

        if (numBitsForXBlocker > 0) {
            byte = CompressIntegers(xVals, boundaryX.first, boundaryX.second, bitstream, bitIndexInByte, byte, "goalblocker X");
        }
        if (numBitsForZBlocker > 0) {
            byte = CompressIntegers(zVals, boundaryZ.first, boundaryZ.second, bitstream, bitIndexInByte, byte, "goalblocker Z");
        }


        byte = CompressBits(data.freezeCar, bitstream, bitIndexInByte, byte, false);

        byte = CompressBits(data.hasStartingJump, bitstream, bitIndexInByte, byte, true);


        char* base64Out = new char[bitstream.size() * 2];
        size_t base64OutLen = 0;
        base64_encode((const char*)bitstream.data(), bitstream.size(), base64Out, &base64OutLen, 0);
        std::string base64Encoded(base64Out, base64OutLen);
        delete[] base64Out;



        outFile.write(base64Encoded.c_str(), base64Encoded.size());

        outFile.close();
    }


}
std::unordered_map<std::string, CustomTrainingData> StorageManager::loadCompressedTrainingData(const std::filesystem::path& trainingPacksFolder) {

    std::unordered_map<std::string, CustomTrainingData> trainingDataMap;

    if (!std::filesystem::exists(trainingPacksFolder)) {
        return trainingDataMap;
    }
    if (!std::filesystem::is_directory(trainingPacksFolder)) {
        LOG("Training packs path is not a directory: {}", trainingPacksFolder.string());
        return trainingDataMap;
    }

    for (const auto& entry : std::filesystem::directory_iterator(trainingPacksFolder)) {
        if (!entry.is_directory()) {
            continue;
        }

        std::filesystem::path packFolder = entry.path();
        std::filesystem::path packFile = packFolder / "trainingpack.txt";

        if (!std::filesystem::exists(packFile)) {
            continue;
        }
        std::ifstream inFile(packFile, std::ios::binary);
        if (!inFile.is_open()) {
            continue;
        }

        std::string line;
        std::getline(inFile, line);
        inFile.close();

        if (line.empty()) continue;

        char* decodedOut = new char[line.size()];
        size_t decodedOutLen = 0;
        base64_decode(line.c_str(), line.size(), decodedOut, &decodedOutLen, 0);
        std::string output(decodedOut, decodedOutLen);
        delete[] decodedOut;
        std::vector<uint8_t> bitstream(output.begin(), output.end());

        if (bitstream.empty()) {
            LOG("Error: Failed to decode base64 line.");
            continue;
        }

        size_t bitIndex = 0;
        bool hasCode = ReadBits(bitstream, bitIndex, TRAINING_CODE_FLAG_BITS) != 0;


        std::string code;
        if (hasCode) {

            code.resize(TRAINING_CODE_CHARS);
            for (size_t i = 0; i < TRAINING_CODE_CHARS; ++i) {
                code[i] = static_cast<char>(ReadBits(bitstream, bitIndex, 8));
            }

        }


        size_t nameLength = ReadBits(bitstream, bitIndex, NAME_LEN_BITS);
        if (nameLength == 0) {
            LOG("Error: Invalid name length in the header.");
            continue;
        }


        size_t numShots = ReadBits(bitstream, bitIndex, NUM_SHOTS_BITS);
        size_t minBoost = ReadBits(bitstream, bitIndex, BOOST_MIN_BITS);
        size_t minVelocity = ReadBits(bitstream, bitIndex, VELOCITY_MIN_BITS);

        size_t maxMagnitude = ReadBits(bitstream, bitIndex, VELOCITY_MIN_BITS);

        size_t maxMagnitudeAng = ReadBits(bitstream, bitIndex, 8);
        size_t minGoalBlockX = ReadBits(bitstream, bitIndex, GOAL_MIN_BITS);
        size_t minGoalBlockZ = ReadBits(bitstream, bitIndex, GOAL_MIN_BITS);

        uint8_t packedBits = (uint8_t)ReadBits(bitstream, bitIndex, 7);
        int numBitsForBoost = (packedBits >> 4) & 0x07;
        int numBitsForVelocity = packedBits & 0x0F;



        uint8_t packedGoalBlockerBits = (uint8_t)ReadBits(bitstream, bitIndex, 8);
        int numBitsForXBlocker = (packedGoalBlockerBits >> 4) & 0x0F;
        int numBitsForZBlocker = packedGoalBlockerBits & 0x0F;


        std::string name(nameLength, '\0');
        for (size_t i = 0; i < nameLength; ++i) {
            name[i] = (char)ReadBits(bitstream, bitIndex, 7);
        }

        CustomTrainingDataflattened trainingData;
        trainingData.initCustomTrainingData((int)numShots, name);

        if (hasCode) {
            trainingData.code = code;
        }

        if (numBitsForBoost == 0) {
            std::fill(trainingData.boostAmounts.begin(), trainingData.boostAmounts.end(), minBoost);
        }
        else {
            DecompressIntegers(trainingData.boostAmounts, minBoost, numBitsForBoost, bitstream, bitIndex, "boost");
        }

        if (numBitsForVelocity == 0) {
            std::fill(trainingData.startingVelocity.begin(), trainingData.startingVelocity.end(), minVelocity);
        }
        else {
            DecompressIntegers(trainingData.startingVelocity, minVelocity, numBitsForVelocity, bitstream, bitIndex, "velocity");
        }

        if (maxMagnitude == 0) {
            std::fill(trainingData.extendedStartingVelocities.begin(), trainingData.extendedStartingVelocities.end(), Vector(0, 0, 0));
        }
        else {
            DecompressVectors(trainingData.extendedStartingVelocities, maxMagnitude, bitstream, bitIndex, "extended starting velocity");
        }

        std::vector<bool> hasAngularVelocity(trainingData.extendedStartingAngularVelocities.size());
        DecompressBits(hasAngularVelocity, bitstream, bitIndex);
        size_t numAngVelocities = 0;
        for (auto hasAng : hasAngularVelocity) {
            if (hasAng) {
                numAngVelocities++;
            }
        }
        if (numAngVelocities > 0) {
            std::vector<Vector> angularVelocities(numAngVelocities);
            DecompressVectors(angularVelocities, maxMagnitudeAng, bitstream, bitIndex, "extended starting angular velocity");
            for (size_t i = 0; i < trainingData.extendedStartingAngularVelocities.size(); i++) {
                if (hasAngularVelocity[i]) {
                    trainingData.extendedStartingAngularVelocities[i] = angularVelocities[i];
                }
                else {
                    trainingData.extendedStartingAngularVelocities[i] = Vector(0, 0, 0);
                }
            }
        }
        else {
            std::fill(trainingData.extendedStartingAngularVelocities.begin(), trainingData.extendedStartingAngularVelocities.end(), Vector(0, 0, 0));
        }



        size_t numGoalBlockers = numShots;
        trainingData.goalBlockers.resize(numGoalBlockers);

        std::vector<int> xVals(numGoalBlockers * 2);
        std::vector<int> zVals(numGoalBlockers * 2);

        if (numBitsForXBlocker == 0) {
            std::fill(xVals.begin(), xVals.end(), minGoalBlockX);
        }
        else {
            DecompressIntegers(xVals, minGoalBlockX, numBitsForXBlocker, bitstream, bitIndex, "x Goal vals");
        }

        if (numBitsForZBlocker == 0) {
            std::fill(zVals.begin(), zVals.end(), minGoalBlockZ);
        }
        else {
            DecompressIntegers(zVals, minGoalBlockZ, numBitsForZBlocker, bitstream, bitIndex, "z Goal vals");
        }

        for (int i = 0; i < numGoalBlockers; i++) {
            trainingData.goalBlockers[i].first.X = (float)xVals[i * 2];
            trainingData.goalBlockers[i].second.X = (float)xVals[i * 2 + 1];
            trainingData.goalBlockers[i].first.Z = (float)zVals[i * 2];
            trainingData.goalBlockers[i].second.Z = (float)zVals[i * 2 + 1];
            trainingData.goalBlockers[i].first.Y = 5140.f;
            trainingData.goalBlockers[i].second.Y = 5140.f;
            if (trainingData.goalBlockers[i].first.X == 910.f && trainingData.goalBlockers[i].first.Z == 20.f && trainingData.goalBlockers[i].second.X == 910.f && trainingData.goalBlockers[i].second.Z == 20.f) {
                trainingData.goalAnchors[i] = { false, false };

            }
            else {

                trainingData.goalAnchors[i] = { true, true };
            }
        }
        DecompressBits(trainingData.freezeCar, bitstream, bitIndex);

        DecompressBits(trainingData.hasStartingJump, bitstream, bitIndex);

        if (!code.empty()) {
            trainingDataMap[code] = trainingData.inflate();
        }
        else {
            trainingDataMap[name] = trainingData.inflate();
        }
    }

    if (trainingDataMap.empty()) {
        LOG("Warning: No valid training data found in the file after decoding.");
    }

    return trainingDataMap;
}