#include "pch.h"
#include "versatileTraining.h"
//
//
//void VersatileTraining::dumpParamsToFile(void* params, size_t length, const std::filesystem::path& filepath) {
//    if (!params || length == 0) {
//        LOG("Invalid params or length");
//        return;
//    }
//
//    std::filesystem::create_directories(filepath.parent_path());
//    std::ofstream out(filepath, std::ios::binary);  // binary mode!
//
//    if (!out) {
//        LOG("Failed to open file: {}", filepath.string());
//        return;
//    }
//
//    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(params);
//    out.write(reinterpret_cast<const char*>(bytes), length);
//
//    LOG("Successfully dumped raw params to {}", filepath.string());
//}
//
//
//std::vector<size_t> VersatileTraining::findStringInHexDump(
//    const std::filesystem::path& filepath, // Change parameter type to std::filesystem::path
//    const std::string& searchStr,
//    bool isUtf16 = true // Set to false for pure ASCII
//) {
//    std::vector<size_t> offsets;
//    std::ifstream file(filepath, std::ios::binary);  // Use the std::filesystem::path directly
//    if (!file) {
//        LOG("Failed to open file: ");
//        return offsets;
//    }
//
//    // Read the entire file into memory
//    std::vector<uint8_t> fileData(
//        (std::istreambuf_iterator<char>(file)),
//        std::istreambuf_iterator<char>()
//    );
//
//    // Convert the search string to the target format (UTF-16LE or ASCII)
//    std::vector<uint8_t> searchPattern;
//    if (isUtf16) {
//        for (char c : searchStr) {
//            searchPattern.push_back(static_cast<uint8_t>(c));
//            searchPattern.push_back(0x00); // UTF-16LE low-byte
//        }
//    }
//    else {
//        searchPattern.assign(searchStr.begin(), searchStr.end());
//    }
//
//    // Search for the pattern in the file data
//    auto it = std::search(
//        fileData.begin(), fileData.end(),
//        searchPattern.begin(), searchPattern.end()
//    );
//
//    while (it != fileData.end()) {
//        size_t offset = std::distance(fileData.begin(), it);
//        offsets.push_back(offset);
//        it = std::search(it + 1, fileData.end(), searchPattern.begin(), searchPattern.end());
//    }
//
//    return offsets;
//}
