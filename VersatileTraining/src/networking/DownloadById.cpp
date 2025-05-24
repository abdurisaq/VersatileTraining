#include <pch.h>
#include "src/core/VersatileTraining.h"
#include "JsonParser.h"

void VersatileTraining::DownloadTrainingPackById(std::string packId) {
    if (packId.empty()) {
         
        return;
    }
     

    CurlRequest req;
    req.url = "https://versatile-training-hub.vercel.app/api/plugin/download/" + packId;
    req.verb = "GET"; 

  
    HttpWrapper::SendCurlRequest(req, [this, packId](int status, std::string responseBody) {
        if (status == 200) {
             
            PackInfo pack  = parsePack(responseBody);
            packInfoToLocalStorage(pack, myDataFolder); 
            *trainingData = storageManager.loadCompressedTrainingDataWithRecordings(myDataFolder); 
            for (auto& [key, value] : *trainingData) {
                shiftToNegative(value);
            }
        }
        else {
             
              // Log error response from server
            
        }
        });
}