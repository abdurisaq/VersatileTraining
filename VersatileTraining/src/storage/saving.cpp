#include "pch.h"
#include "src/storage/StorageManager.h"
#include <cmath>
#include <iostream>

constexpr size_t NAME_LEN_BITS = 5;
constexpr size_t NUM_SHOTS_BITS = 6;
constexpr size_t BOOST_MIN_BITS = 7;
constexpr size_t VELOCITY_MIN_BITS = 13;
constexpr size_t GOAL_MIN_BITS = 12;
constexpr size_t TRAINING_CODE_FLAG_BITS = 1;
constexpr size_t TRAINING_CODE_CHARS = 19;

inline static size_t CalculateRequiredBits(int range) {
    if (range <= 0) return 0;
    return (size_t)std::floor(std::log2(range)) + 1;
}

uint8_t WriteBits(
    size_t value,
    size_t numBits,
    std::vector<uint8_t>& bitstream,
    size_t& bitIndexInByte,
    uint8_t currentByte
) {
    for (size_t i = 0; i < numBits; ++i) {
        bool bit = (value >> (numBits - 1 - i)) & 1;
        if (bit) {
            currentByte |= (1 << (7 - bitIndexInByte));
        }
        bitIndexInByte++;

        if (bitIndexInByte == 8) {
            bitstream.push_back(currentByte);
            currentByte = 0;
            bitIndexInByte = 0;
        }
    }
    return currentByte;
}

size_t ReadBits(const std::vector<uint8_t>& bitstream, size_t& bitIndex, size_t numBits) {
    size_t value = 0;
    size_t byteIndex = bitIndex / 8;
    size_t bitOffset = bitIndex % 8;

    for (size_t i = 0; i < numBits; ++i) {
        if (bitstream[byteIndex] & (1 << (7 - bitOffset))) {
            value |= (1ULL << (numBits - 1 - i));
        }

        bitOffset++;
        if (bitOffset == 8) {
            bitOffset = 0;
            byteIndex++;
        }
    }

    bitIndex += numBits;
    return value;
}

//helpers
uint8_t CompressVectors(
    const std::vector<Vector>& vectors,
    float maxMagnitude,
    std::vector<uint8_t>& bitstream,
    size_t& bitIndexInByte,
    uint8_t currentByte
) {
    const int numBits = 16;
    const float range = 2.0f * maxMagnitude;

    for (const Vector& v : vectors) {
        for (float value : {v.X, v.Y, v.Z}) {
            float clamped = std::clamp(value, -maxMagnitude, maxMagnitude);
            float normalized = (clamped + maxMagnitude) / range;

            size_t quantized = static_cast<size_t>(std::round(normalized * ((1 << numBits) - 1)));

            currentByte = WriteBits(quantized, numBits, bitstream, bitIndexInByte, currentByte);
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
    uint8_t currentByte
) {
    int range = globalMax - globalMin;
    size_t numBits = CalculateRequiredBits(range);

    if (numBits == 0) return currentByte;

    for (int value : values) {
        size_t compressed = static_cast<size_t>(value - globalMin);
        currentByte = WriteBits(compressed, numBits, bitstream, bitIndexInByte, currentByte);
    }
    return currentByte;
}

uint8_t CompressBits(
    const std::vector<bool>& bools,
    std::vector<uint8_t>& bitstream,
    size_t& bitIndexInByte,
    uint8_t currentByte
) {
    for (bool b : bools) {
        currentByte = WriteBits(b ? 1 : 0, 1, bitstream, bitIndexInByte, currentByte);
    }
    return currentByte;
}

void DecompressVectors(
    std::vector<Vector>& output,
    float maxMagnitude,
    const std::vector<uint8_t>& bitstream,
    size_t& bitIndex
) {
    int numBits = 16;
    const float range = 2.0f * maxMagnitude;
    const float scale = 1.0f / ((1 << numBits) - 1);

    for (size_t i = 0; i < output.size(); ++i) {
        size_t xBits = ReadBits(bitstream, bitIndex, numBits);
        size_t yBits = ReadBits(bitstream, bitIndex, numBits);
        size_t zBits = ReadBits(bitstream, bitIndex, numBits);

        output[i].X = (xBits * scale * range) - maxMagnitude;
        output[i].Y = (yBits * scale * range) - maxMagnitude;
        output[i].Z = (zBits * scale * range) - maxMagnitude;
    }
}

void DecompressIntegers(
    std::vector<int>& output,
    int globalMin,
    int numBits,
    const std::vector<uint8_t>& bitstream,
    size_t& bitIndex
) {
    for (size_t i = 0; i < output.size(); ++i) {
        size_t compressed = ReadBits(bitstream, bitIndex, numBits);
        output[i] = (int)(compressed + globalMin);
    }
}

void DecompressBits(std::vector<bool>& vec, const std::vector<uint8_t>& bitstream, size_t& bitIndex) {
    for (size_t i = 0; i < vec.size(); ++i) {
        size_t bit = ReadBits(bitstream, bitIndex, 1);
        vec[i] = (bit != 0);
    }
}

std::pair<int, int> getMinMaxAmount(const std::vector<int>& arr) {
    if (arr.empty()) return { 0, 0 };
    int min = arr[0];
    int max = arr[0];
    for (int val : arr) {
        if (val < min) min = val;
        if (val > max) max = val;
    }
    return { min, max };
}



void StorageManager::saveCompressedTrainingData(const std::unordered_map<std::string, CustomTrainingData>& trainingData, const std::filesystem::path& trainingPacksFolder) {

    std::filesystem::create_directories(trainingPacksFolder);

    for (std::pair<std::string, CustomTrainingData>  entry : trainingData) {
        const CustomTrainingDataflattened& data = entry.second.deflate();

        std::string folderName = !data.code.empty() ? data.code : data.name;
        for (char& c : folderName) {
            if (strchr("<>:\"/\\|?*", c)) c = '_';
        }

        std::filesystem::path packFolder = trainingPacksFolder / folderName;
        std::filesystem::create_directories(packFolder);
        std::filesystem::path packFile = packFolder / "trainingpack.txt";

        std::ofstream outFile(packFile, std::ios::binary);
        if (!outFile.is_open()) continue;

        std::vector<uint8_t> bitstream;
        uint8_t byte = 0;
        size_t bitIndexInByte = 0;

        bool hasCode = !data.code.empty() && data.code.length() == TRAINING_CODE_CHARS;
        byte = WriteBits(hasCode ? 1 : 0, TRAINING_CODE_FLAG_BITS, bitstream, bitIndexInByte, byte);

        if (hasCode) {
            for (char c : data.code) {
                byte = WriteBits(static_cast<size_t>(c), 8, bitstream, bitIndexInByte, byte);
            }
        }

        byte = WriteBits(data.name.size() & 0xFF, NAME_LEN_BITS, bitstream, bitIndexInByte, byte);

        byte = WriteBits(data.numShots & 0xFF, NUM_SHOTS_BITS, bitstream, bitIndexInByte, byte);

        std::pair<int, int> boundaryBoosts = getMinMaxAmount(data.boostAmounts);
        byte = WriteBits(boundaryBoosts.first & 0xFF, BOOST_MIN_BITS, bitstream, bitIndexInByte, byte);

        std::pair<int, int> boundaryVelocities = getMinMaxAmount(data.startingVelocity);
        byte = WriteBits((boundaryVelocities.first >> 5) & 0xFF, 8, bitstream, bitIndexInByte, byte); // High 8
        byte = WriteBits(boundaryVelocities.first & 0x1F, 5, bitstream, bitIndexInByte, byte);       // Low 5 Total 13

        std::vector<int> magnitudes;
        for (const auto& vec : data.extendedStartingVelocities) {
            magnitudes.push_back(static_cast<int>(vec.magnitude()));
        }
        std::pair<int, int> magnitudeBounds = getMinMaxAmount(magnitudes);
        byte = WriteBits((magnitudeBounds.second >> 5) & 0xFF, 8, bitstream, bitIndexInByte, byte);
        byte = WriteBits(magnitudeBounds.second & 0x1F, 5, bitstream, bitIndexInByte, byte);

        std::vector<int> magnitudesAng;
        for (const auto& vec : data.extendedStartingAngularVelocities) {
            magnitudesAng.push_back(static_cast<int>(vec.magnitude()));
        }
        std::pair<int, int> magnitudeBoundsAng = getMinMaxAmount(magnitudesAng);
        magnitudeBoundsAng.first++; 
        magnitudeBoundsAng.second++;
        byte = WriteBits(magnitudeBoundsAng.second & 0xFF, 8, bitstream, bitIndexInByte, byte);


        std::vector<int> xVals, zVals;
        xVals.reserve(data.goalBlockers.size() * 2);
        zVals.reserve(data.goalBlockers.size() * 2);

        for (const auto& [first, second] : data.goalBlockers) {
            xVals.push_back(static_cast<int>(first.X));
            xVals.push_back(static_cast<int>(second.X));
            zVals.push_back(static_cast<int>(first.Z));
            zVals.push_back(static_cast<int>(second.Z));
        }
        std::pair<int, int> boundaryX = getMinMaxAmount(xVals);
        byte = WriteBits((boundaryX.first >> 4) & 0xFF, 8, bitstream, bitIndexInByte, byte);
        byte = WriteBits(boundaryX.first & 0xF, 4, bitstream, bitIndexInByte, byte); // Total 12 bits

        std::pair<int, int> boundaryZ = getMinMaxAmount(zVals);
        byte = WriteBits((boundaryZ.first >> 4) & 0xFF, 8, bitstream, bitIndexInByte, byte);
        byte = WriteBits(boundaryZ.first & 0xF, 4, bitstream, bitIndexInByte, byte);

        size_t numBitsForBoost = CalculateRequiredBits(boundaryBoosts.second - boundaryBoosts.first);
        size_t numBitsForVelocity = CalculateRequiredBits(boundaryVelocities.second - boundaryVelocities.first);
        size_t numBitsForXBlocker = CalculateRequiredBits(boundaryX.second - boundaryX.first);
        size_t numBitsForZBlocker = CalculateRequiredBits(boundaryZ.second - boundaryZ.first);

        byte = WriteBits(numBitsForBoost, 3, bitstream, bitIndexInByte, byte);
        byte = WriteBits(numBitsForVelocity, 4, bitstream, bitIndexInByte, byte);
        byte = WriteBits(numBitsForXBlocker, 4, bitstream, bitIndexInByte, byte);
        byte = WriteBits(numBitsForZBlocker, 4, bitstream, bitIndexInByte, byte);

        for (char c : data.name) {
            byte = WriteBits(static_cast<size_t>(c), 7, bitstream, bitIndexInByte, byte);
        }

        if (numBitsForBoost > 0) {
            byte = CompressIntegers(data.boostAmounts, boundaryBoosts.first, boundaryBoosts.second, bitstream, bitIndexInByte, byte);
        }
        if (numBitsForVelocity > 0) {
            byte = CompressIntegers(data.startingVelocity, boundaryVelocities.first, boundaryVelocities.second, bitstream, bitIndexInByte, byte);
        }

        byte = CompressVectors(data.extendedStartingVelocities, magnitudeBounds.second, bitstream, bitIndexInByte, byte);

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

        byte = CompressBits(hasAngularVelocity, bitstream, bitIndexInByte, byte);
        byte = CompressVectors(angularVelocitiesToSave, magnitudeBoundsAng.second, bitstream, bitIndexInByte, byte);

        if (numBitsForXBlocker > 0) {
            byte = CompressIntegers(xVals, boundaryX.first, boundaryX.second, bitstream, bitIndexInByte, byte);
        }
        if (numBitsForZBlocker > 0) {
            byte = CompressIntegers(zVals, boundaryZ.first, boundaryZ.second, bitstream, bitIndexInByte, byte);
        }

        byte = CompressBits(data.freezeCar, bitstream, bitIndexInByte, byte);
        byte = CompressBits(data.hasStartingJump, bitstream, bitIndexInByte, byte);


        if (bitIndexInByte > 0) {
            bitstream.push_back(byte);
        }


        char* base64Out = new char[bitstream.size() * 2 + 1]; 
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

    if (!std::filesystem::exists(trainingPacksFolder) || !std::filesystem::is_directory(trainingPacksFolder)) {
        return trainingDataMap;
    }

    for (const auto& entry : std::filesystem::directory_iterator(trainingPacksFolder)) {
        if (!entry.is_directory()) continue;

        std::filesystem::path packFile = entry.path() / "trainingpack.txt";
        if (!std::filesystem::exists(packFile)) continue;

        std::ifstream inFile(packFile, std::ios::binary);
        if (!inFile.is_open()) continue;

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

        if (bitstream.empty()) continue;

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
        if (nameLength == 0) continue;

        size_t numShots = ReadBits(bitstream, bitIndex, NUM_SHOTS_BITS);
        size_t minBoost = ReadBits(bitstream, bitIndex, BOOST_MIN_BITS);

        size_t minVelocity = ReadBits(bitstream, bitIndex, 8) << 5;
        minVelocity |= ReadBits(bitstream, bitIndex, 5);


        size_t maxMagnitude = ReadBits(bitstream, bitIndex, 8) << 5;
        maxMagnitude |= ReadBits(bitstream, bitIndex, 5);

        size_t maxMagnitudeAng = ReadBits(bitstream, bitIndex, 8);


        size_t minGoalBlockX = ReadBits(bitstream, bitIndex, 8) << 4;
        minGoalBlockX |= ReadBits(bitstream, bitIndex, 4);

        size_t minGoalBlockZ = ReadBits(bitstream, bitIndex, 8) << 4;
        minGoalBlockZ |= ReadBits(bitstream, bitIndex, 4);

        size_t numBitsForBoost = ReadBits(bitstream, bitIndex, 3);
        size_t numBitsForVelocity = ReadBits(bitstream, bitIndex, 4);
        size_t numBitsForXBlocker = ReadBits(bitstream, bitIndex, 4);
        size_t numBitsForZBlocker = ReadBits(bitstream, bitIndex, 4);

        std::string name(nameLength, '\0');
        for (size_t i = 0; i < nameLength; ++i) {
            name[i] = (char)ReadBits(bitstream, bitIndex, 7);
        }

        CustomTrainingDataflattened trainingData;
        trainingData.initCustomTrainingData((int)numShots, name);
        if (hasCode) trainingData.code = code;

        if (numBitsForBoost == 0) {
            std::fill(trainingData.boostAmounts.begin(), trainingData.boostAmounts.end(), minBoost);
        }
        else {
            DecompressIntegers(trainingData.boostAmounts, (int)minBoost, (int)numBitsForBoost, bitstream, bitIndex);
        }

        if (numBitsForVelocity == 0) {
            std::fill(trainingData.startingVelocity.begin(), trainingData.startingVelocity.end(), minVelocity);
        }
        else {
            DecompressIntegers(trainingData.startingVelocity, (int)minVelocity, (int)numBitsForVelocity, bitstream, bitIndex);
        }

        if (maxMagnitude == 0) {
            std::fill(trainingData.extendedStartingVelocities.begin(), trainingData.extendedStartingVelocities.end(), Vector(0, 0, 0));
        }
        else {
            DecompressVectors(trainingData.extendedStartingVelocities, (float)maxMagnitude, bitstream, bitIndex);
        }

        std::vector<bool> hasAngularVelocity(trainingData.extendedStartingAngularVelocities.size());
        DecompressBits(hasAngularVelocity, bitstream, bitIndex);

        size_t numAngVelocities = 0;
        for (bool b : hasAngularVelocity) if (b) numAngVelocities++;

        if (numAngVelocities > 0) {
            std::vector<Vector> angularVelocities(numAngVelocities);
            DecompressVectors(angularVelocities, (float)maxMagnitudeAng, bitstream, bitIndex);

            size_t angIdx = 0;
            for (size_t i = 0; i < trainingData.extendedStartingAngularVelocities.size(); i++) {
                if (hasAngularVelocity[i]) {
                    trainingData.extendedStartingAngularVelocities[i] = angularVelocities[angIdx++];
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
            DecompressIntegers(xVals, (int)minGoalBlockX, (int)numBitsForXBlocker, bitstream, bitIndex);
        }

        if (numBitsForZBlocker == 0) {
            std::fill(zVals.begin(), zVals.end(), minGoalBlockZ);
        }
        else {
            DecompressIntegers(zVals, (int)minGoalBlockZ, (int)numBitsForZBlocker, bitstream, bitIndex);
        }

        for (int i = 0; i < numGoalBlockers; i++) {
            trainingData.goalBlockers[i].first.X = (float)xVals[i * 2];
            trainingData.goalBlockers[i].second.X = (float)xVals[i * 2 + 1];
            trainingData.goalBlockers[i].first.Z = (float)zVals[i * 2];
            trainingData.goalBlockers[i].second.Z = (float)zVals[i * 2 + 1];
            trainingData.goalBlockers[i].first.Y = 5140.f;
            trainingData.goalBlockers[i].second.Y = 5140.f;

            if (trainingData.goalBlockers[i].first.X == 910.f && trainingData.goalBlockers[i].first.Z == 20.f &&
                trainingData.goalBlockers[i].second.X == 910.f && trainingData.goalBlockers[i].second.Z == 20.f) {
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

    return trainingDataMap;
}