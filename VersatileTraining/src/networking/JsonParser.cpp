#include "pch.h"
#include "JsonParser.h"


std::string extractValue(const std::string& str, const std::string& key) {
    auto start = str.find("\"" + key + "\":");
    if (start == std::string::npos) return "";
    start += key.size() + 3;

    if (str[start] == '"') {
        ++start;
        auto end = str.find('"', start);
        return str.substr(start, end - start);
    }
    else if (str[start] == 'n') { // null
        return "";
    }
    else {
        auto end = str.find_first_of(",}", start);
        return str.substr(start, end - start);
    }
}

// Helper to extract array of strings (very basic)
std::vector<std::string> extractArray(const std::string& str, const std::string& key) {
    std::vector<std::string> result;
    auto start = str.find("\"" + key + "\":[");
    if (start == std::string::npos) return result;
    start = str.find('[', start) + 1;
    auto end = str.find(']', start);
    std::string arrayContent = str.substr(start, end - start);

    std::istringstream ss(arrayContent);
    std::string item;
    while (std::getline(ss, item, ',')) {
        size_t firstQuote = item.find('"');
        size_t lastQuote = item.rfind('"');
        if (firstQuote != std::string::npos && lastQuote != std::string::npos && lastQuote > firstQuote) {
            result.push_back(item.substr(firstQuote + 1, lastQuote - firstQuote - 1));
        }
    }
    return result;
}

PackInfo parsePack(const std::string& json) {
    PackInfo info;
    info.id = extractValue(json, "id");
    info.name = extractValue(json, "name");
    info.description = extractValue(json, "description");
    info.code = extractValue(json, "code");
    info.difficulty = std::stoi(extractValue(json, "difficulty"));
    info.tags = extractArray(json, "tags");
    info.totalShots = std::stoi(extractValue(json, "totalShots"));
    info.packMetadataCompressed = extractValue(json, "packMetadataCompressed");
    info.recordingDataCompressed = extractValue(json, "recordingDataCompressed");

    std::replace(info.recordingDataCompressed.begin(), info.recordingDataCompressed.end(), '.', '\n');
    return info;
}


void packInfoToLocalStorage(const PackInfo packInfo,std::filesystem::path dataFolder ,std::atomic<bool>& hasAction,
    std::string& pendingAction,
    std::mutex& pendingActionMutex) {
	std::filesystem::path trainingDataPath = dataFolder / "TrainingPacks";
	std::filesystem::create_directories(trainingDataPath);
    std::filesystem::path packPath = trainingDataPath / (packInfo.code);
    
    std::filesystem::create_directories(packPath);

    std::filesystem::path customTrainingDataPath = packPath / "trainingpack.txt";

    std::ofstream file(customTrainingDataPath);
    if (file.is_open()) {
		file<< packInfo.packMetadataCompressed;
        file.close();
	}
    else {
         
    }
    std::filesystem::path recordingDataPath = packPath / "shots.rec";
    std::ofstream recordingFile(recordingDataPath);
    if (recordingFile.is_open()) {
		recordingFile << packInfo.recordingDataCompressed;
		recordingFile.close();
	}
    else {
		 
	}

    std::filesystem::path metaDataPath = packPath / "metadata.json";
    std::ofstream metaDataFile(metaDataPath);
    


    if (metaDataFile.is_open()) {
		metaDataFile << "{\n";
        metaDataFile << "  \"id\": \"" << packInfo.id;
        metaDataFile << "\",";
        metaDataFile << "\"name\": \"" << packInfo.name;
        metaDataFile << "\",";
        metaDataFile << "\"shots\": " << packInfo.totalShots;
        metaDataFile << "\n}";

		metaDataFile.close();
	}
    else {
		 
	}


    
}


void packInfoToLocalStorage(const PackInfo packInfo, std::filesystem::path dataFolder) {
    std::filesystem::path trainingDataPath = dataFolder / "TrainingPacks";
    std::filesystem::create_directories(trainingDataPath);
    std::filesystem::path packPath = trainingDataPath / (packInfo.code);

    std::filesystem::create_directories(packPath);

    std::filesystem::path customTrainingDataPath = packPath / "trainingpack.txt";

    std::ofstream file(customTrainingDataPath);
    if (file.is_open()) {
        file << packInfo.packMetadataCompressed;
        file.close();
    }
    else {
         
    }
    std::filesystem::path recordingDataPath = packPath / "shots.rec";
    std::ofstream recordingFile(recordingDataPath);
    if (recordingFile.is_open()) {
        recordingFile << packInfo.recordingDataCompressed;
        recordingFile.close();
    }
    else {
         
    }

    std::filesystem::path metaDataPath = packPath / "metadata.json";
    std::ofstream metaDataFile(metaDataPath);



    if (metaDataFile.is_open()) {
        metaDataFile << "{\n";
        metaDataFile << "  \"id\": \"" << packInfo.id;
        metaDataFile << "\",";
        metaDataFile << "\"name\": \"" << packInfo.name;
        metaDataFile << "\",";
        metaDataFile << "\"shots\": " << packInfo.totalShots;
        metaDataFile << "\n}";

        metaDataFile.close();
    }
    else {
         
    }


    
}