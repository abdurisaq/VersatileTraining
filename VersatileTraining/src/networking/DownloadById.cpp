#include <pch.h>
#include "src/core/VersatileTraining.h"
#include "JsonParser.h"
// You might need to include a JSON parsing library if you use one, e.g., nlohmann/json
// #include <nlohmann/json.hpp> 

void VersatileTraining::DownloadTrainingPackById(std::string packId) {
    if (packId.empty()) {
        LOG("Invalid pack ID provided for download.");
        return;
    }
    LOG("Attempting to download training pack with ID: {}", packId);

    CurlRequest req;
    req.url = "http://localhost:3000/api/plugin/download/" + packId;
    req.verb = "GET"; 

  
    HttpWrapper::SendCurlRequest(req, [this, packId](int status, std::string responseBody) {
        if (status == 200) {
            LOG("Successfully received response for pack ID {}. HTTP status: {}. Response size: {}", packId, status, responseBody.size());
            PackInfo pack  = parsePack(responseBody);
            packInfoToLocalStorage(pack, myDataFolder); 
            *trainingData = storageManager.loadCompressedTrainingDataWithRecordings(myDataFolder); 
        }
        else {
            LOG("Error downloading pack ID {}: HTTP status {}", packId, status);
            LOG("Error response body: {}", responseBody); // Log error response from server
            
        }
        });
}