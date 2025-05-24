#include "pch.h"
#include "RecordingStorage.h"

std::vector<float> delta_encode(const std::vector<float>& data) {
    std::vector<float> deltas;
    if (data.empty()) return deltas;
    deltas.push_back(data[0]);
    for (size_t i = 1; i < data.size(); ++i)
        deltas.push_back(data[i] - data[i - 1]);
    return deltas;
}

// bit pack bools
std::vector<uint8_t> bitpack_bools(const std::vector<std::vector<bool>>& bool_cols) {
    std::vector<uint8_t> packed;
    size_t row_count = bool_cols[0].size();
    for (size_t i = 0; i < row_count; ++i) {
        uint8_t byte = 0;
        for (size_t b = 0; b < bool_cols.size(); ++b) {
            if (bool_cols[b][i])
                byte |= (1 << b);
        }
        packed.push_back(byte);
    }
    return packed;
}

// encoding before compression
std::vector<uint8_t> RecordingStorage::encode_recordings(const std::vector<ShotRecording>& recordings) {
    std::vector<float> throttle, steer, pitch, yaw, roll, dodgeForward, dodgeStrafe;
    std::vector<std::vector<bool>> bools(5); // [Handbrake, Jump, ActivateBoost, HoldingBoost, Jumped]

    for (const auto& rec : recordings) {
        for (const auto& input : rec.inputs) {
            throttle.push_back(input.Throttle);
            steer.push_back(input.Steer);
            pitch.push_back(input.Pitch);
            yaw.push_back(input.Yaw);
            roll.push_back(input.Roll);
            dodgeForward.push_back(input.DodgeForward);
            dodgeStrafe.push_back(input.DodgeStrafe);

            bools[0].push_back(input.Handbrake);
            bools[1].push_back(input.Jump);
            bools[2].push_back(input.ActivateBoost);
            bools[3].push_back(input.HoldingBoost);
            bools[4].push_back(input.Jumped);
        }
    }

    std::vector<uint8_t> buffer;

    auto write_floats = [&](const std::vector<float>& vals) {
        auto deltas = delta_encode(vals);
        buffer.insert(buffer.end(), reinterpret_cast<const uint8_t*>(deltas.data()),
            reinterpret_cast<const uint8_t*>(deltas.data() + deltas.size()));
        };

    write_floats(throttle);
    write_floats(steer);
    write_floats(pitch);
    write_floats(yaw);
    write_floats(roll);
    write_floats(dodgeForward);
    write_floats(dodgeStrafe);

    auto packed_bools = bitpack_bools(bools);
    buffer.insert(buffer.end(), packed_bools.begin(), packed_bools.end());

    return buffer;
}

// zlib
std::vector<uint8_t> RecordingStorage::compress_data(const std::vector<uint8_t>& data) {
    

    size_t originalSizeOfData = data.size();

    uLongf compressed_size = compressBound(data.size());

    
    std::vector<uint8_t> result(sizeof(uLong) + compressed_size);

    uLong originalSize = data.size();
    memcpy(result.data(), &originalSize, sizeof(uLong));

    if (compress(result.data() + sizeof(uLong), &compressed_size, data.data(), data.size()) != Z_OK) {
         
        return {};
    }
    float ratio = originalSizeOfData > 0 ? (float)compressed_size / originalSizeOfData : 0.0f;
    LOG("Compression stats: original size: {} bytes, compressed size: {} bytes, ratio: {:.2f}%",
        originalSizeOfData, compressed_size, ratio * 100.0f);

    result.resize(sizeof(uLong) + compressed_size);
    return result;
}
