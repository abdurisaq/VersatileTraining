#include "pch.h"
#include "src/storage/StorageManager.h"

void writeString(std::ofstream& out, const std::string& str) {
    size_t len = str.size();
    out.write(reinterpret_cast<const char*>(&len), sizeof(len));
    out.write(str.c_str(), len);
}

std::string readString(std::ifstream& in) {
    size_t len;
    in.read(reinterpret_cast<char*>(&len), sizeof(len));
    std::string str(len, '\0');
    in.read(&str[0], len);
    return str;
}

template<typename T>
void writeBinary(std::ofstream& out, const T& val) {
    out.write(reinterpret_cast<const char*>(&val), sizeof(T));
}

template<typename T>
void readBinary(std::ifstream& in, T& val) {
    in.read(reinterpret_cast<char*>(&val), sizeof(T));
}


void StorageManager::saveReplayStates(const std::vector<ReplayState>& replays, const std::filesystem::path& fileName) {
    std::filesystem::create_directories(fileName.parent_path());
    std::ofstream outFile(fileName, std::ios::binary);
    if (!outFile) {
        throw std::runtime_error("Failed to open file for writing");
    }

    size_t count = replays.size();
    writeBinary(outFile, count);

    for (const auto& r : replays) {
        writeString(outFile, r.replayName);
        writeString(outFile, r.formattedTimeStampOfSaved);
        writeString(outFile, r.replayTime);
        writeString(outFile, r.timeRemainingInGame);
        writeString(outFile, r.focusPlayerName);

        writeBinary(outFile, static_cast<int>(r.captureSource));

        writeBinary(outFile, r.carVelocity);
        writeBinary(outFile, r.carAngularVelocity);
        writeBinary(outFile, r.carLocation);
        writeBinary(outFile, r.carRotation);

        writeBinary(outFile, r.boostAmount);
        writeBinary(outFile, r.jumpTimer);
        writeBinary(outFile, r.hasJump);
        writeBinary(outFile, r.boosting);

        writeBinary(outFile, r.ballLocation);
        writeBinary(outFile, r.ballSpeed);
        writeBinary(outFile, r.ballRotation);

        writeBinary(outFile, r.ballSet);
        writeBinary(outFile, r.carLocationSet);
        writeBinary(outFile, r.carRotationSet);
    }
}


std::vector<ReplayState> StorageManager::loadReplayStates(const std::filesystem::path& fileName) {
    std::ifstream inFile(fileName, std::ios::binary);
    if (!inFile) {
        throw std::runtime_error("Failed to open file for reading");
    }

    size_t count;
    readBinary(inFile, count);
    std::vector<ReplayState> replays(count);

    for (auto& r : replays) {
        r.replayName = readString(inFile);
        r.formattedTimeStampOfSaved = readString(inFile);
        r.replayTime = readString(inFile);
        r.timeRemainingInGame = readString(inFile);
        r.focusPlayerName = readString(inFile);

        int captureSourceInt;
        readBinary(inFile, captureSourceInt);
        r.captureSource = static_cast<CaptureSource>(captureSourceInt);

        readBinary(inFile, r.carVelocity);
        readBinary(inFile, r.carAngularVelocity);
        readBinary(inFile, r.carLocation);
        readBinary(inFile, r.carRotation);

        readBinary(inFile, r.boostAmount);
        readBinary(inFile, r.jumpTimer);
        readBinary(inFile, r.hasJump);
        readBinary(inFile, r.boosting);

        readBinary(inFile, r.ballLocation);
        readBinary(inFile, r.ballSpeed);
        readBinary(inFile, r.ballRotation);

        readBinary(inFile, r.ballSet);
        readBinary(inFile, r.carLocationSet);
        readBinary(inFile, r.carRotationSet);
    }

    return replays;
}
