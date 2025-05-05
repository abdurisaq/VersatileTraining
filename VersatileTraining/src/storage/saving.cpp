#include "pch.h"
#include "src/core/versatileTraining.h"
#include "include/base64.h"

constexpr size_t NAME_LEN_BITS = 5;
constexpr size_t NUM_SHOTS_BITS = 6;
constexpr size_t BOOST_MIN_BITS = 7;
constexpr size_t VELOCITY_MIN_BITS = 12;
constexpr size_t GOAL_X_MIN_BITS = 11;
constexpr size_t GOAL_Z_MIN_BITS = 10;
constexpr size_t NUM_FLOAT_BITS = 16;
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

    if (!debugLabel.empty()) {
        LOG("Compressing {} with {} bits per float and max magnitude {}", debugLabel, numBits, maxMagnitude);
    }

    for (const Vector& v : vectors) {
        for (float value : {v.X, v.Y, v.Z}) {
            float clamped = std::clamp(value, -maxMagnitude, maxMagnitude);
            float normalized = (clamped + maxMagnitude) / range;
            uint32_t quantized = static_cast<uint32_t>(std::round(normalized * ((1 << numBits) - 1)));
            LOG("original Value : {}", value);
            LOG("quantized value being saved : {}", quantized);
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
    if (!debugLabel.empty()) {
        LOG("num bits for {}: {}", debugLabel, numBits);
    }

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

uint8_t CompressBits(const std::vector<bool>& freezeCar, std::vector<uint8_t>& bitstream,size_t & bitCount, uint8_t currentByte, bool finalWrite) {
    
    //uint8_t currentByte = 0;
    

    for (bool freeze : freezeCar) {
        if (freeze) {
            currentByte |= (1 <<(7- bitCount));  
        }
        bitCount++;

        //dont change
        if (bitCount == 8) {
            bitstream.push_back(currentByte);
            currentByte = 0;
            bitCount = 0;
        }
    }
    if (finalWrite) {
        bitstream.push_back(currentByte);
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

    int numBits = 16;//CalculateRequiredBits(maxMagnitude*2)
    const float range = 2.0f * maxMagnitude;
    const float scale = 1.0f / ((1 << numBits) - 1);

    if (!debugLabel.empty()) {
        LOG("Decompressing {} with {} bits per float and max magnitude {}", debugLabel, numBits, maxMagnitude);
    }

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
    if (!debugLabel.empty()) {
        LOG("Decompressing {} with {} bits and global min {}", debugLabel, numBits, globalMin);
    }

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
std::pair<int,int> getMinMaxAmount(std::vector<int> arr) {
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
	
uint8_t writeBits(uint8_t byte, size_t& bitIndex, std::vector<uint8_t>& bitstream, const std::vector<uint8_t>&data, int numBitsToWrite) {
	

    bool lastByte = false;
    for (int i = 0; i < numBitsToWrite; i++) {

        if(!lastByte && (i ==0 || i % 8 ==0) && (numBitsToWrite - i <= 8)) {
            lastByte = true;
        }
        bool bit = (data[i / 8] >> (7-(i % 8))) & 1;
        if (lastByte) {
            bit = (data[i / 8] & (1 << (numBitsToWrite - i - 1)));
        }
		byte |= (bit <<  (7-bitIndex));

		bitIndex++;

		
        if (bitIndex == 8) {
			bitstream.push_back(byte);
			byte = 0;  
			bitIndex = 0; 
		}
	
    }
    return byte;

}

void VersatileTraining::SaveCompressedTrainingData(const std::unordered_map<std::string, CustomTrainingData>& trainingData, const std::filesystem::path& fileName) {
    std::filesystem::create_directories(fileName.parent_path());
    std::ofstream outFile(fileName, std::ios::binary);

    
    for (std::pair<std::string,CustomTrainingData> entry : trainingData) {
        //const CustomTrainingData& data = entry.second;
         const CustomTrainingDataflattened& data = entry.second.deflate();
         
  
        // update starting velocities, get rid of negatives by shifting the values up 2000
        
       
        // Header: nameLength  | numShots | minBoost | minVelocity | boostbitAmount | velocitybitAmount | name
        size_t nameLength = data.name.size();
        std::vector<uint8_t> bitstream;
        uint8_t byte = 0;
        size_t bitIndexInByte = 0;
        std::vector<uint8_t> binaryDataToWrite;
        binaryDataToWrite.push_back(nameLength & 0xFF);  
        LOG("writing name length : {} to file", nameLength & 0xFF);
        byte = writeBits(byte, bitIndexInByte, bitstream, binaryDataToWrite, NAME_LEN_BITS);//max 30 character name
        //bitstream.push_back((nameLength >> 8) & 0xFF);  
        //bitstream.push_back(nameLength & 0xFF);         

        binaryDataToWrite.clear();
        binaryDataToWrite.push_back(data.numShots & 0xFF);
        LOG("writing num shots : {} to file", data.numShots & 0xFF);
        byte = writeBits(byte, bitIndexInByte, bitstream, binaryDataToWrite, NUM_SHOTS_BITS);
        


       
        std::pair<int,int> boundaryBoosts = getMinMaxAmount(data.boostAmounts);
        std::pair<int,int> boundaryVelocities = getMinMaxAmount(data.startingVelocity);
       

        binaryDataToWrite.clear();
        binaryDataToWrite.push_back(boundaryBoosts.first & 0xFF);
        LOG("writing min boost : {} to file", boundaryBoosts.first & 0xFF);
        byte = writeBits(byte, bitIndexInByte, bitstream, binaryDataToWrite, BOOST_MIN_BITS);

        binaryDataToWrite.clear();
        binaryDataToWrite.push_back((boundaryVelocities.first >>4) & 0xFF);
        binaryDataToWrite.push_back((boundaryVelocities.first) & 0xF);
        LOG("min velocity {}", boundaryVelocities.first);
        LOG("writing min velocity : {} to file", (boundaryVelocities.first>>4) & 0xFF);
        LOG("writing min velocity : {} to file", ((boundaryVelocities.first) & 0xF));
        byte = writeBits(byte, bitIndexInByte, bitstream, binaryDataToWrite, VELOCITY_MIN_BITS);

        
        std::vector<int> magnitudes;
        for (const auto& vec : data.extendedStartingVelocities) {
            int magnitude = static_cast<int>(vec.magnitude());
            LOG("extended starting velocity magnitude: {}", magnitude);
            magnitudes.push_back(magnitude);
        }
        std::pair<int, int> magnitudeBounds = getMinMaxAmount(magnitudes);

        binaryDataToWrite.clear();
        binaryDataToWrite.push_back((magnitudeBounds.second >> 4) & 0xFF);            // Top 8 bits (bits 12 to 5)
        binaryDataToWrite.push_back((magnitudeBounds.second & 0xF));            // Lower 5 bits (bits 4 to 0), shifted into high bits of next byte

        LOG("writing 12-bit value: {}", magnitudeBounds.second);
        LOG("byte 1: {:#010b}", binaryDataToWrite[0]);
        LOG("byte 2: {:#010b}", binaryDataToWrite[1]);
        byte = writeBits(byte, bitIndexInByte, bitstream, binaryDataToWrite, 12); // 13 bits for the magnitude

        std::vector<int> xVals;
        std::vector<int> zVals;

        xVals.reserve(data.goalBlockers.size()*2);
        zVals.reserve(data.goalBlockers.size()*2);

        for (const auto& [first, second] : data.goalBlockers) {
            xVals.push_back(static_cast<int>(first.X));
            xVals.push_back(static_cast<int>(second.X));
            zVals.push_back(static_cast<int>(first.Z));
            zVals.push_back(static_cast<int>(second.Z));
        }
        std::pair<int,int> boundaryX = getMinMaxAmount(xVals);
        

        binaryDataToWrite.clear();
        binaryDataToWrite.push_back((boundaryX.first >> 4) & 0xFF);
        binaryDataToWrite.push_back((boundaryX.first) & 0xF);
        LOG("min goalblocker X: {}", boundaryX.first);
        LOG("writing min goal blocker X : {} to file", (boundaryX.first >> 4) & 0xFF);
        LOG("writing min goal blocker X : {} to file", ((boundaryX.first) & 0xF));
        byte = writeBits(byte, bitIndexInByte, bitstream, binaryDataToWrite, VELOCITY_MIN_BITS);

        std::pair<int, int> boundaryZ = getMinMaxAmount(zVals);

        binaryDataToWrite.clear();
        binaryDataToWrite.push_back((boundaryZ.first >> 4) & 0xFF);
        binaryDataToWrite.push_back((boundaryZ.first) & 0xF);
        LOG("min goalblocker Z: {}", boundaryZ.first);
        LOG("writing min goal blocker Z : {} to file", (boundaryZ.first >> 4) & 0xFF);
        LOG("writing min goal blocker Z : {} to file", ((boundaryZ.first) & 0xF));
        byte = writeBits(byte, bitIndexInByte, bitstream, binaryDataToWrite, VELOCITY_MIN_BITS);

        //size_t numBitsForBoost = CalculateRequiredBits(boundaryBoosts.second - boundaryBoosts.first);
        //size_t numBitsForVelocity = CalculateRequiredBits(boundaryVelocities.second - boundaryVelocities.first);

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
        LOG("writing num bits for boost : {} to file", numBitsForBoost & 0xFF);
        byte = writeBits(byte, bitIndexInByte, bitstream, binaryDataToWrite, 3);

        binaryDataToWrite.clear();
        binaryDataToWrite.push_back(numBitsForVelocity & 0xFF);

        LOG("writing num bits for velocity : {} to file", numBitsForVelocity & 0xFF);
        byte = writeBits(byte, bitIndexInByte, bitstream, binaryDataToWrite, 4);

        

        


        binaryDataToWrite.clear();
        binaryDataToWrite.push_back(numBitsForXBlocker & 0xFF);
        LOG("writing num bits for goal blocker X : {} to file.  minimum {}, max {}", numBitsForXBlocker & 0xFF,boundaryX.first,boundaryX.second);
        byte = writeBits(byte, bitIndexInByte, bitstream, binaryDataToWrite, 4);

        binaryDataToWrite.clear();
        binaryDataToWrite.push_back(numBitsForZBlocker & 0xFF);
        LOG("writing num bits for goal blocker Z : {} to file. minimum {}, max {}", numBitsForZBlocker & 0xFF,boundaryZ.first,boundaryZ.second);
        byte = writeBits(byte, bitIndexInByte, bitstream, binaryDataToWrite, 4);

        LOG("num bits for boost : {}", numBitsForBoost);
        LOG("num bits for velocity : {}", numBitsForVelocity);
        LOG("num bits for goal blocker X : {}", numBitsForXBlocker);
        LOG("num bits for goal blocker Z : {}", numBitsForZBlocker);
       
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

        for (int val : xVals) {
            LOG("x vals writing : {}", val);
        }
        for (int val : zVals) {
            LOG("z vals writing: {}", val);
        }
        if (numBitsForXBlocker > 0) {
			byte = CompressIntegers(xVals, boundaryX.first, boundaryX.second, bitstream, bitIndexInByte, byte, "goalblocker X");
		}
        if (numBitsForZBlocker > 0) {
			byte = CompressIntegers(zVals, boundaryZ.first, boundaryZ.second, bitstream, bitIndexInByte, byte, "goalblocker Z");
		}


        byte = CompressBits(data.freezeCar, bitstream, bitIndexInByte, byte,false);
        
        byte = CompressBits(data.hasStartingJump, bitstream, bitIndexInByte, byte,true);
      

        // base64
        std::string base64Encoded = base64_encode(bitstream.data(), (int)bitstream.size());

        
        outFile.write(base64Encoded.c_str(), base64Encoded.size());

        
        outFile.write("\n", 1);  // just for cleanliness
    }

    outFile.close();
}
std::unordered_map<std::string, CustomTrainingData> VersatileTraining::LoadCompressedTrainingData(const std::filesystem::path& fileName) {
    LOG("loading from file");
    std::ifstream inFile(fileName, std::ios::binary);
    if (!inFile.is_open()) {
        LOG("Error: Failed to open file for reading: ");
        return {};
    }

    std::string line;
    std::unordered_map<std::string, CustomTrainingData> trainingDataMap;

    while (std::getline(inFile, line)) {
        if (line.empty()) continue;

        std::vector<uint8_t> bitstream = base64_decode_bytearr(line);
        if (bitstream.empty()) {
            LOG("Error: Failed to decode base64 line.");
            continue;
        }

        size_t bitIndex = 0;

        size_t nameLength = ReadBits(bitstream, bitIndex, NAME_LEN_BITS);
        if (nameLength == 0) {
            LOG("Error: Invalid name length in the header.");
            continue;
        }
        LOG("Name length : {}", nameLength);

        size_t numShots = ReadBits(bitstream, bitIndex, NUM_SHOTS_BITS);
        LOG("numShots : {}", numShots);
        size_t minBoost = ReadBits(bitstream, bitIndex, BOOST_MIN_BITS);
        LOG("minBoost : {}", minBoost);
        size_t minVelocity = ReadBits(bitstream, bitIndex, VELOCITY_MIN_BITS);
        LOG("minVelocity : {}", minVelocity);

        size_t maxMagnitude = ReadBits(bitstream, bitIndex, 12);
        LOG("maxMagnitude : {}", maxMagnitude);


        size_t minGoalBlockX = ReadBits(bitstream, bitIndex, VELOCITY_MIN_BITS);
        LOG("min goalblocker X : {}", minGoalBlockX);
        size_t minGoalBlockZ = ReadBits(bitstream, bitIndex, VELOCITY_MIN_BITS);
        LOG("min goalblocker Z : {}", minGoalBlockZ);

        uint8_t packedBits = (uint8_t)ReadBits(bitstream, bitIndex, 7);
        int numBitsForBoost = (packedBits >> 4) & 0x07;
        int numBitsForVelocity = packedBits & 0x0F;
        LOG("num bits for boost : {}", numBitsForBoost);
        LOG("num bits for velocity : {}", numBitsForVelocity);

        

        uint8_t packedGoalBlockerBits =(uint8_t) ReadBits(bitstream, bitIndex, 8);
        int numBitsForXBlocker = (packedGoalBlockerBits >> 4) & 0x0F;
        int numBitsForZBlocker = packedGoalBlockerBits & 0x0F;

        LOG("num bits for goal blocker X : {}", numBitsForXBlocker);
        LOG("num bits for goal blocker Z : {}", numBitsForZBlocker);

        std::string name(nameLength, '\0');
        for (size_t i = 0; i < nameLength; ++i) {
            name[i] =(char) ReadBits(bitstream, bitIndex, 7);
        }
        LOG("name : {}", name);

        CustomTrainingDataflattened trainingData;
        trainingData.initCustomTrainingData((int)numShots, name);

        if (numBitsForBoost == 0) {
            std::fill(trainingData.boostAmounts.begin(), trainingData.boostAmounts.end(), minBoost);
        }
        else {
            DecompressIntegers(trainingData.boostAmounts, minBoost, numBitsForBoost, bitstream, bitIndex,"boost");
        }

        for (int boostAmounts : trainingData.boostAmounts) {
            LOG("boostAmounts : {}", boostAmounts);
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
        size_t numGoalBlockers = numShots; // or whatever your logic should be
        trainingData.goalBlockers.resize(numGoalBlockers);

        std::vector<int> xVals(numGoalBlockers * 2);
        std::vector<int> zVals(numGoalBlockers * 2);

        // Then decompress into these vectors
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
        for (int val : xVals) {
			LOG("x vals reading: {}", val);
		}
        for (int val : zVals) {
            LOG("z vals reading: {}", val);
        }
        // Now populate the goal blockers
        for (int i = 0; i < numGoalBlockers; i++) {
            trainingData.goalBlockers[i].first.X = (float)xVals[i * 2];
            trainingData.goalBlockers[i].second.X = (float)xVals[i * 2 + 1];
            trainingData.goalBlockers[i].first.Z = (float)zVals[i * 2];
            trainingData.goalBlockers[i].second.Z = (float)zVals[i * 2 + 1];
            trainingData.goalBlockers[i].first.Y = 5140.f;
            trainingData.goalBlockers[i].second.Y = 5140.f;
            if (trainingData.goalBlockers[i].first.X == 910.f && trainingData.goalBlockers[i].first.Z == 20.f && trainingData.goalBlockers[i].second.X == 910.f && trainingData.goalBlockers[i].second.Z == 20.f) {
				trainingData.goalAnchors[i] = { false, false };
                LOG("setting anchors to false");
            }
            else {
                LOG("setting anchor to trues");
                trainingData.goalAnchors[i] ={ true, true };
            }
        }
        for (int startingVelocity : trainingData.startingVelocity) {
            LOG("startingVelocity : {}", startingVelocity);
        }

        DecompressBits(trainingData.freezeCar, bitstream, bitIndex);
        for (bool freeze : trainingData.freezeCar) {
            LOG("freeze : {}", freeze);
        }
        DecompressBits(trainingData.hasStartingJump, bitstream, bitIndex);
        for (bool jump : trainingData.hasStartingJump) {
			LOG("jump : {}", jump);
		}

        LOG("loading training pack with name : {}", name);
        trainingDataMap[name] = trainingData.inflate();
    }

    if (trainingDataMap.empty()) {
        LOG("Warning: No valid training data found in the file after decoding.");
    }

    return trainingDataMap;
}