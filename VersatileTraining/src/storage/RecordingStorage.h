#pragma once
#include "pch.h"



class RecordingStorage {
public:
    void saveAllRecordings(const std::unordered_map<std::string, CustomTrainingData>& trainingData,
        const std::filesystem::path& basePath);

    void loadAllRecordings(std::unordered_map<std::string, CustomTrainingData>& trainingData,
        const std::filesystem::path& basePath);

private:
    // helper functions for saving/loading
    void savePackRecordings(const std::string& packId, const CustomTrainingData& data,
        const std::filesystem::path& packFolder);

    
    void loadPackRecordings(const std::string& packId, CustomTrainingData& data,
        const std::filesystem::path& packFolder);

    std::vector<uint8_t> compressShotRecording(const ShotRecording& recording);
    ShotRecording decompressShotRecording(const std::vector<uint8_t>& compressedData);

    
    std::filesystem::path getRecordingPath(const std::filesystem::path& packFolder, int shotIndex);

    std::vector<uint8_t> encode_recordings(const std::vector<ShotRecording>& recordings);

    std::vector<uint8_t> compress_data(const std::vector<uint8_t>& data);

    std::string sanitizeFilename(const std::string& input);

    std::vector<uint8_t> decompress_data(const std::vector<uint8_t>& compressed);

    std::vector<ShotRecording> decode_recordings(const std::vector<uint8_t>& data, uint32_t inputCount);


    std::vector<float> delta_decode(const std::vector<float>& deltas);
    std::vector<std::vector<bool>> bitunpack_bools(const std::vector<uint8_t>& packed, size_t num_cols, size_t row_count);
};