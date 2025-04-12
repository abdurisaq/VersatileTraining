#include "pch.h"
#include "versatileTraining.h"
#include "base64.h"

constexpr size_t NAME_LEN_BITS = 5;
constexpr size_t NUM_SHOTS_BITS = 6;
constexpr size_t BOOST_MIN_BITS = 7;
constexpr size_t VELOCITY_MIN_BITS = 12;


inline static size_t CalculateRequiredBits(int range) {
    return (size_t)std::ceil(std::log2(range));
}


uint8_t CompressBoost(const std::vector<int>& boostAmounts, int globalMinBoost, int globalMaxBoost, std::vector<uint8_t>& bitstream,size_t& bitIndexInByte, uint8_t currentByte) {
    int rangeBoost = globalMaxBoost - globalMinBoost;  
    size_t bitsForBoost = CalculateRequiredBits(rangeBoost);  
    LOG("num bits for boost : {}", bitsForBoost);

    //uint8_t currentByte = 0;
    //size_t bitIndexInByte = 0; 

    for (int boost : boostAmounts) {
        int compressedBoost = boost - globalMinBoost;  

        for (size_t i = 0; i < bitsForBoost; ++i) {
            bool bit = (compressedBoost >> (bitsForBoost - 1 - i)) & 1;
            currentByte |= (bit << (7 - bitIndexInByte)); 
            bitIndexInByte++;  

            
            if (bitIndexInByte == 8) {
                bitstream.push_back(currentByte);
                currentByte = 0;  
                bitIndexInByte = 0;  //
            }
        }
    }

    return currentByte;
}
uint8_t CompressStartingVelocity(const std::vector<int>& startingVelocity, int globalMinVelocity, int globalMaxVelocity, std::vector<uint8_t>& bitstream, size_t& bitIndexInByte, uint8_t currentByte) {
    int rangeVelocity = globalMaxVelocity -globalMinVelocity;  
    size_t bitsForVelocity = CalculateRequiredBits(rangeVelocity);  
    LOG("num bits for velocity : {}",bitsForVelocity);
    //uint8_t currentByte = 0;
    //size_t bitIndexInByte = 0; 

    for (int velocity : startingVelocity) {
        int compressedVelocity = velocity - globalMinVelocity;  

        
        for (size_t i = 0; i < bitsForVelocity; ++i) {
            
            bool bit = (compressedVelocity >> (bitsForVelocity - 1 - i)) & 1;
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

void CompressFreeze(const std::vector<bool>& freezeCar, std::vector<uint8_t>& bitstream,size_t & bitCount, uint8_t currentByte) {
    
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

    if (bitCount > 0) {
        bitstream.push_back(currentByte);
    }
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


void DecompressBoost(std::vector<int>& boostAmounts, int globalMinBoost,int bitsForBoost, std::vector<uint8_t>& bitstream, size_t& bitIndex) {
    
    for (size_t i = 0; i < boostAmounts.size(); ++i) {
        int compressedBoost = ReadBits(bitstream, bitIndex, bitsForBoost);
        boostAmounts[i] = compressedBoost + globalMinBoost;  
    }
}


void DecompressStartingVelocity(std::vector<int>& startingVelocity, int globalMinVelocity, int bitsForVelocity, std::vector<uint8_t>& bitstream, size_t& bitIndex) {
    
    LOG("when decompressing starting velocity, global min velocity is : {} and numBitsForVelocity is : {}", globalMinVelocity, bitsForVelocity);
    for (size_t i = 0; i < startingVelocity.size(); ++i) {
        int compressedVelocity = ReadBits(bitstream, bitIndex, bitsForVelocity);
        LOG("read for velocity : {}", compressedVelocity);
        startingVelocity[i] = compressedVelocity + globalMinVelocity;  
    }
}


void DecompressFreeze(std::vector<bool>& freezeCar, std::vector<uint8_t>& bitstream, size_t& bitIndex) {
    size_t bitCount = 0;
    for (size_t i = 0; i < freezeCar.size(); ++i) {
        size_t bit = ReadBits(bitstream, bitIndex, 1);  
        freezeCar[i] = (bit != 0);  
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

    
    for (const auto& entry : trainingData) {
        const CustomTrainingData& data = entry.second;
        
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


        size_t numBitsForBoost = CalculateRequiredBits(boundaryBoosts.second - boundaryBoosts.first);
        size_t numBitsForVelocity = CalculateRequiredBits(boundaryVelocities.second - boundaryVelocities.first);

        binaryDataToWrite.clear();
        binaryDataToWrite.push_back(numBitsForBoost & 0xFF);
        LOG("writing num bits for boost : {} to file", numBitsForBoost & 0xFF);
        byte = writeBits(byte, bitIndexInByte, bitstream, binaryDataToWrite, 3);

        binaryDataToWrite.clear();
        binaryDataToWrite.push_back(numBitsForVelocity & 0xFF);

        LOG("writing num bits for velocity : {} to file", numBitsForVelocity & 0xFF);
        byte = writeBits(byte, bitIndexInByte, bitstream, binaryDataToWrite, 4);


        LOG("num bits for boost : {}", numBitsForBoost);
        LOG("num bits for velocity : {}", numBitsForVelocity);
       
        for (char c : data.name) {
            std::vector<uint8_t> charBits = { static_cast<uint8_t>(c) };
            byte = writeBits(byte, bitIndexInByte, bitstream, charBits, 7);
        }

        // Compress 
        byte = CompressBoost(data.boostAmounts, boundaryBoosts.first, boundaryBoosts.second, bitstream, bitIndexInByte, byte);
      
        byte =CompressStartingVelocity(data.startingVelocity, boundaryVelocities.first, boundaryVelocities.second, bitstream, bitIndexInByte, byte);
     
        CompressFreeze(data.freezeCar, bitstream, bitIndexInByte, byte);
      
      

        // base64
        std::string base64Encoded = base64_encode(bitstream.data(), bitstream.size());

        
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

    std::string base64EncodedData((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
    inFile.close();
    
    if (base64EncodedData.empty()) {
        LOG("Warning: The file is empty." );
        return {}; 
    }
    
    std::vector<uint8_t> bitstream = base64_decode_bytearr(base64EncodedData);
    
    if (bitstream.empty()) {
        LOG("Error: Failed to decode base64 data.");
        return {};  
    }

    size_t bitIndex = 0;

    
    std::unordered_map<std::string, CustomTrainingData> trainingDataMap;
    LOG("do we get here 3");
    
    while (bitIndex < bitstream.size()) {
        // Read header (nameLength | name | numShots | minBoost | minVelocity)
        size_t nameLength = ReadBits(bitstream, bitIndex, NAME_LEN_BITS); 
        if (nameLength == 0) {
            LOG("Error: Invalid name length in the header.");
            break;  
        }
        LOG("Name length : {}", nameLength);
        
        
        size_t numShots = ReadBits(bitstream, bitIndex, NUM_SHOTS_BITS);  
        LOG("numShots : {}", numShots);
        int minBoost = ReadBits(bitstream, bitIndex, BOOST_MIN_BITS);  
        LOG("minBoost : {}", minBoost);
        int minVelocity = ReadBits(bitstream, bitIndex, VELOCITY_MIN_BITS);  
        LOG("minVelocity : {}", minVelocity);
        uint8_t packedBits = ReadBits(bitstream, bitIndex, 7); 
        int numBitsForBoost = (packedBits >> 4) & 0x07;  
        int numBitsForVelocity = packedBits & 0x0F;  
        LOG("num bits for boost : {}", numBitsForBoost);
        LOG("num bits for velocity : {}", numBitsForVelocity);

        std::string name(nameLength, '\0');

        for (size_t i = 0; i < nameLength; ++i) {
            name[i] = ReadBits(bitstream, bitIndex, 7);  
        }
        LOG("name : {}", name);

        
        CustomTrainingData trainingData;
        trainingData.initCustomTrainingData(numShots, name);




        DecompressBoost(trainingData.boostAmounts, minBoost, numBitsForBoost, bitstream, bitIndex);
        for (int boostAmounts : trainingData.boostAmounts) {
            LOG("boostAmounts : {}", boostAmounts);
        }
        DecompressStartingVelocity(trainingData.startingVelocity, minVelocity, numBitsForVelocity, bitstream, bitIndex);
        for (int startingVelocity : trainingData.startingVelocity) {
			LOG("startingVelocity : {}", startingVelocity);
		}
        DecompressFreeze(trainingData.freezeCar, bitstream, bitIndex);
        for (bool freeze : trainingData.freezeCar) {
            LOG("freeze : {}", freeze);
        }
       
        trainingDataMap[name] = trainingData;
    }

    
    if (trainingDataMap.empty()) {
        LOG("Warning: No valid training data found in the file after decoding.");
    }

    return trainingDataMap;
}