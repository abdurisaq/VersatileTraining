
#include "pch.h"
#include "RecordingStorage.h"

std::vector<float> RecordingStorage::delta_decode(const std::vector<float>& deltas) {
    std::vector<float> values;
    if (deltas.empty()) return values;

    values.resize(deltas.size());
    values[0] = deltas[0];
    for (size_t i = 1; i < deltas.size(); ++i) {
        values[i] = values[i - 1] + deltas[i];
    }
    return values;
}

std::vector<std::vector<bool>> RecordingStorage::bitunpack_bools(const std::vector<uint8_t>& packed, size_t num_cols, size_t row_count) {
    std::vector<std::vector<bool>> result(num_cols, std::vector<bool>(row_count));
    for (size_t i = 0; i < row_count; ++i) {
        uint8_t byte = packed[i];
        for (size_t b = 0; b < num_cols; ++b) {
            result[b][i] = (byte & (1 << b)) != 0;
        }
    }
    return result;
}

std::vector<uint8_t> RecordingStorage::decompress_data(const std::vector<uint8_t>& compressed) {
    //gotta be bigger than header size
    if (compressed.size() < sizeof(uLong)) {
         
        return {};
    }

    uLong uncompressedSize;
    memcpy(&uncompressedSize, compressed.data(), sizeof(uLong));

    LOG("Decompressing data: {} compressed bytes, expected uncompressed size: {}",
        compressed.size(), uncompressedSize);

    // no world it goes over 10mb, so throw error if so, really should be 1mb error throwing, but change later
    if (uncompressedSize > 10 * 1024 * 1024) { 
         
        return {};
    }

    std::vector<uint8_t> result(uncompressedSize);
    uLongf actualSize = uncompressedSize;

    int status = uncompress(
        result.data(), &actualSize,
        compressed.data() + sizeof(uLong),
        compressed.size() - sizeof(uLong)
    );

    if (status != Z_OK) {
        LOG("Decompression failed with status: {}. Buffer sizes: compressed={}, header={}, uncompressed={}",
            status, compressed.size(), sizeof(uLong), uncompressedSize);
        return {};
    }

    result.resize(actualSize);
    return result;
}

std::vector<ShotRecording> RecordingStorage::decode_recordings(const std::vector<uint8_t>& data, uint32_t inputCount) {
    if (data.empty() || inputCount == 0) {
         
        return { ShotRecording() };
    }

     

    std::vector<ShotRecording> result(1);
    result[0].inputs.resize(inputCount);
    const size_t float_size = sizeof(float);

    
    size_t min_required_size = inputCount * float_size * 7; 
    size_t bool_data_size = inputCount; 

    if (data.size() < min_required_size + bool_data_size) {
        LOG("ERROR: Data size {} less than required size {} for {} inputs",
            data.size(), min_required_size + bool_data_size, inputCount);
        return result;
    }

    
    auto read_floats = [&](size_t offset, size_t count) {
        std::vector<float> deltas(count);
        memcpy(deltas.data(), data.data() + offset, count * float_size);
        return delta_decode(deltas);
        };

    size_t offset = 0;
    std::vector<float> throttle = read_floats(offset, inputCount);
    offset += inputCount * float_size;

    std::vector<float> steer = read_floats(offset, inputCount);
    offset += inputCount * float_size;

    std::vector<float> pitch = read_floats(offset, inputCount);
    offset += inputCount * float_size;

    std::vector<float> yaw = read_floats(offset, inputCount);
    offset += inputCount * float_size;

    std::vector<float> roll = read_floats(offset, inputCount);
    offset += inputCount * float_size;

    std::vector<float> dodgeForward = read_floats(offset, inputCount);
    offset += inputCount * float_size;

    std::vector<float> dodgeStrafe = read_floats(offset, inputCount);
    offset += inputCount * float_size;

    
    std::vector<uint8_t> packed_bools(data.data() + offset, data.data() + offset + inputCount);
    std::vector<std::vector<bool>> bool_cols = bitunpack_bools(packed_bools, 5, inputCount);

    // controller inputs needed to recreate
    for (size_t i = 0; i < inputCount; ++i) {
        ControllerInput& input = result[0].inputs[i];

       
        if (i < throttle.size()) input.Throttle = throttle[i];
        if (i < steer.size()) input.Steer = steer[i];
        if (i < pitch.size()) input.Pitch = pitch[i];
        if (i < yaw.size()) input.Yaw = yaw[i];
        if (i < roll.size()) input.Roll = roll[i];
        if (i < dodgeForward.size()) input.DodgeForward = dodgeForward[i];
        if (i < dodgeStrafe.size()) input.DodgeStrafe = dodgeStrafe[i];

        
        if (i < bool_cols[0].size()) input.Handbrake = bool_cols[0][i];
        if (i < bool_cols[1].size()) input.Jump = bool_cols[1][i];
        if (i < bool_cols[2].size()) input.ActivateBoost = bool_cols[2][i];
        if (i < bool_cols[3].size()) input.HoldingBoost = bool_cols[3][i];
        if (i < bool_cols[4].size()) input.Jumped = bool_cols[4][i];
    }

    return result;
}