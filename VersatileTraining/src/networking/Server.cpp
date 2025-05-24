
#include "pch.h"
#include "src/core/VersatileTraining.h"
#include "src/networking/JsonParser.h"
#pragma comment(lib, "ws2_32.lib")


const int SERVER_PORT = 7437;
const std::string AUTH_TOKEN = "versatile_training_scanner_token";
const std::string PLAYER_ID_FILE = "player_id.txt";

// Global mutex for thread safety
std::mutex g_mutex;

// Forward declaration
std::string loadPlayerId();
std::string extractAuthToken(const std::string& request);
std::string extractHttpMethod(const std::string& request);
std::string extractEndpoint(const std::string& request);
std::string extractRequestBody(const std::string& request);
std::string createJsonResponse(int statusCode, const std::string& content, bool includeHeaders = true);
std::string handleOptionsRequest();
std::string handleStatusRequest(const std::string& authToken);
std::string handleLoadPackRequest(const std::string& authToken, const std::string& body, std::filesystem::path dataFolder, std::atomic<bool>& hasAction,
    std::string& pendingAction,
    std::mutex& pendingActionMutex);
std::string handleListPacksRequest(const std::string& authToken, const std::shared_ptr<std::unordered_map<std::string, CustomTrainingData>>& trainingDataPtr);
std::string handlePackDetailsRequest(const std::string& authToken, const std::string& packId, const std::shared_ptr<std::unordered_map<std::string, CustomTrainingData>>& trainingDataPtr);
std::string handlePackRecordingRequest(
    const std::string& authToken,
    const std::string& packId,
    const std::filesystem::path& dataFolder,
    const std::shared_ptr<std::unordered_map<std::string, CustomTrainingData>>& trainingDataPtr);
void handleClientConnection(SOCKET clientSocket, const std::shared_ptr<std::unordered_map<std::string, CustomTrainingData>>& trainingDataPtr, const std::filesystem::path& dataFolder, std::atomic<bool>& hasAction, std::string& pendingAction,
    std::mutex& pendingActionMutex);

std::string id;
std::string base64TrainingData;

std::filesystem::path myDataFolder;



void VersatileTraining::runServer(
    std::atomic<bool>* isRunning,
    std::string playerId,
    std::shared_ptr<std::unordered_map<std::string, CustomTrainingData>> trainingDataPtr,
    std::filesystem::path dataFolder,
    std::atomic<bool>& hasAction,
    std::string & pendingAction,
    std::mutex& pendingActionMutex) {

    id = playerId;

    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
         
        return;
    }

    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
         
        WSACleanup();
        return;
    }

    
    u_long mode = 1; //nonblocking
    if (ioctlsocket(listenSocket, FIONBIO, &mode) == SOCKET_ERROR) {
         
        closesocket(listenSocket);
        WSACleanup();
        return;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(SERVER_PORT);

    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
         
        closesocket(listenSocket);
        WSACleanup();
        return;
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
         
        closesocket(listenSocket);
        WSACleanup();
        return;
    }

     

    std::vector<std::thread> clientThreads;

    while (*isRunning) {
        SOCKET clientSocket = accept(listenSocket, NULL, NULL);

        
        if (clientSocket != INVALID_SOCKET) {
            clientThreads.push_back(std::thread(handleClientConnection, clientSocket, trainingDataPtr, dataFolder,std::ref(hasAction), std::ref(pendingAction), std::ref(pendingActionMutex)));
        }
        else {
            if (WSAGetLastError() == WSAEWOULDBLOCK) {
                Sleep(100);  
            }
            else {
                if (*isRunning) {
                     
                }
                Sleep(100);
            }
        }

        for (auto it = clientThreads.begin(); it != clientThreads.end();) {
            if (it->joinable()) {
                it->join();
                it = clientThreads.erase(it);
            }
            else {
                ++it;
            }
        }
    }

     
    closesocket(listenSocket);

    for (auto& thread : clientThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    WSACleanup();
     
}

void handleClientConnection(
    SOCKET clientSocket,
    const std::shared_ptr<std::unordered_map<std::string, CustomTrainingData>>& trainingDataPtr,
    const std::filesystem::path& dataFolder, std::atomic<bool> &hasAction,
    std::string& pendingAction,
    std::mutex& pendingActionMutex) {

    char buffer[327680] = { 0 };
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

    if (bytesReceived > 0) {
        
        buffer[bytesReceived] = '\0';
        std::string request(buffer);

        std::string method = extractHttpMethod(request);
        std::string endpoint = extractEndpoint(request);
        std::string authToken = extractAuthToken(request);
        std::string response;

        {
            std::lock_guard<std::mutex> lock(g_mutex);
            if (endpoint != "/status") {
                 
            }
        }

        if (method == "OPTIONS") {
            response = handleOptionsRequest();
        }
        else if (endpoint == "/status" && method == "GET") {
            response = handleStatusRequest(authToken);
        }
        else if (endpoint == "/load-pack" && method == "POST") {
            std::string body = extractRequestBody(request);
            response = handleLoadPackRequest(authToken, body,dataFolder,hasAction,pendingAction,pendingActionMutex);


        }
        else if (endpoint == "/list-packs" && method == "GET") {
            response = handleListPacksRequest(authToken, trainingDataPtr);
        }
        else if (endpoint.find("/pack-details/") == 0 && method == "GET") {
            std::string packId = endpoint.substr(14); // Remove "/pack-details/"
            response = handlePackDetailsRequest(authToken, packId, trainingDataPtr);
        }
        else if (endpoint.find("/pack-recording/") == 0 && method == "GET") {
            
            std::string packId = endpoint.substr(15); 

            
            if (!packId.empty() && (packId[0] == '/' || packId[0] == '\\')) {
                packId = packId.substr(1);
            }

             
            response = handlePackRecordingRequest(authToken, packId, dataFolder, trainingDataPtr);
        }
        else {
            // 404 
            response = createJsonResponse(404, "{\n\"error\": \"Not found\"\n}");
        }

        // Send 
        send(clientSocket, response.c_str(), (int)response.length(), 0);
    }

    
    closesocket(clientSocket);
}


std::string loadPlayerId() {
    return id;
}

std::string extractAuthToken(const std::string& request) {
    size_t authPos = request.find("Authorization: Bearer ");
    if (authPos != std::string::npos) {
        size_t tokenStart = authPos + 22; // Length of "Authorization: Bearer "
        size_t tokenEnd = request.find("\r\n", tokenStart);
        if (tokenEnd != std::string::npos) {
            return request.substr(tokenStart, tokenEnd - tokenStart);
        }
    }
    return "";
}

std::string extractHttpMethod(const std::string& request) {
    size_t methodEnd = request.find(' ');
    if (methodEnd != std::string::npos) {
        return request.substr(0, methodEnd);
    }
    return "";
}

std::string extractEndpoint(const std::string& request) {
    size_t pathStart = request.find(' ') + 1;
    size_t pathEnd = request.find(' ', pathStart);
    if (pathStart != std::string::npos && pathEnd != std::string::npos) {
        std::string path = request.substr(pathStart, pathEnd - pathStart);
        // Remove query parameters if present
        size_t queryPos = path.find('?');
        if (queryPos != std::string::npos) {
            return path.substr(0, queryPos);
        }
        return path;
    }
    return "";
}

std::string extractRequestBody(const std::string& request) {
    size_t bodyStart = request.find("\r\n\r\n");
    if (bodyStart != std::string::npos) {
        return request.substr(bodyStart + 4);
    }
    return "";
}

std::string createJsonResponse(int statusCode, const std::string& content, bool includeHeaders) {
    std::string response;
    if (includeHeaders) {
        response = "HTTP/1.1 " + std::to_string(statusCode) + " " +
            (statusCode == 200 ? "OK" : "Error") + "\r\n";
        response += "Content-Type: application/json\r\n";
        response += "Access-Control-Allow-Origin: *\r\n";
        response += "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
        response += "Access-Control-Allow-Headers: Content-Type, Authorization\r\n";
        response += "Access-Control-Max-Age: 86400\r\n";
        response += "Content-Length: " + std::to_string(content.length()) + "\r\n";
        response += "\r\n";
    }
    response += content;

    return response;
}

std::string handleOptionsRequest() {
    std::string response = "HTTP/1.1 204 No Content\r\n";
    response += "Access-Control-Allow-Origin: *\r\n";
    response += "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n";
    response += "Access-Control-Allow-Headers: Content-Type, Authorization\r\n";
    response += "Access-Control-Max-Age: 86400\r\n";
    response += "\r\n";
    return response;
}

std::string handleStatusRequest(const std::string& authToken) {
    std::string playerId = loadPlayerId();

    if (authToken == AUTH_TOKEN) {
        
        return createJsonResponse(200,
            "{\n"
            "  \"status\": \"plugin_active\",\n"
            "  \"player_id\": \"" + playerId + "\",\n"
            "  \"version\": \"1.0.0\"\n"
            "}");
    }
    else {
        
         
        return createJsonResponse(200,
            "{\n"
            "  \"status\": \"plugin_active\"\n"
            "}");
    }
}

std::string handleLoadPackRequest(const std::string& authToken, const std::string& body, std::filesystem::path dataFolder, std::atomic<bool>& hasAction,
    std::string& pendingAction,
    std::mutex& pendingActionMutex) {
     
     
    if (authToken != AUTH_TOKEN) {
        return createJsonResponse(401,
            "{\n"
            "  \"error\": \"Unauthorized\"\n"
            "}");
    }

    PackInfo pack = parsePack(body);
    
    packInfoToLocalStorage(pack, dataFolder, hasAction, pendingAction,pendingActionMutex);
    
    pack.print();


    {
        std::lock_guard<std::mutex> lock(pendingActionMutex);
        pendingAction = pack.code;
        hasAction.store(true, std::memory_order_release);
    }
    return createJsonResponse(200,
        "{\n"
        "  \"status\": \"success\",\n"
        "  \"message\": \"Training pack loaded successfully\"\n"
        "}");
}

std::string handleListPacksRequest(
    const std::string& authToken,
    const std::shared_ptr<std::unordered_map<std::string, CustomTrainingData>>& trainingDataPtr) {

    if (authToken != AUTH_TOKEN) {
        return createJsonResponse(401, "{\n\"error\": \"Unauthorized\"\n}");
    }

    if (!trainingDataPtr || trainingDataPtr->empty()) {
         
        return createJsonResponse(200, "{\n\"packs\": []\n}");
    }

    std::ostringstream json;
    json << "{\n  \"packs\": [\n";

    bool firstPack = true;
    for (const auto& [packId, pack] : *trainingDataPtr) {
        if (pack.code.empty())continue;
        if (!firstPack) {
            json << ",\n";
        }
        firstPack = false;

        json << "    {\n"
            << "      \"id\": \"" << packId << "\",\n"
            << "      \"name\": \"" << pack.name << "\",\n"
            << "      \"numShots\": " << pack.numShots << "\n"
            << "    }";
    }

    json << "\n  ]\n}";

     
    return createJsonResponse(200, json.str());
}

// Handler to get detailed information about a specific pack
std::string handlePackDetailsRequest(
    const std::string& authToken,
    const std::string& packId,
    const std::shared_ptr<std::unordered_map<std::string, CustomTrainingData>>& trainingDataPtr) {

    if (authToken != AUTH_TOKEN) {
        return createJsonResponse(401, "{\n\"error\": \"Unauthorized\"\n}");
    }

    if (!trainingDataPtr) {
        return createJsonResponse(404, "{\n\"error\": \"No training packs available\"\n}");
    }

    auto it = trainingDataPtr->find(packId);
    if (it == trainingDataPtr->end()) {
         
        return createJsonResponse(404, "{\n\"error\": \"Pack not found\"\n}");
    }

    const CustomTrainingData& pack = it->second;

    
    std::ostringstream json;
    json << "{\n"
        << "  \"id\": \"" << packId << "\",\n"
        << "  \"name\": \"" << pack.name << "\",\n"
        << "  \"code\": \"" << pack.code << "\",\n"
        << "  \"numShots\": " << pack.numShots << ",\n"
        << "  \"currentEditedShot\": " << pack.currentEditedShot << ",\n"
        << "  \"customPack\": " << (pack.customPack ? "true" : "false") << ",\n"
        << "  \"shots\": [\n";

    for (size_t i = 0; i < pack.shots.size(); i++) {
        if (i > 0) {
            json << ",\n";
        }

        const ShotState& shot = pack.shots[i];
        json << "    {\n"
            << "      \"shotIndex\": " << shot.shotIndex << ",\n"
            << "      \"boostAmount\": " << shot.boostAmount << ",\n"
            << "      \"freezeCar\": " << (shot.freezeCar ? "true" : "false") << ",\n"
            << "      \"hasJump\": " << (shot.hasJump ? "true" : "false") << ",\n"
            << "      \"startingVelocity\": " << shot.startingVelocity << ",\n"
            << "      \"hasRecording\": " << (shot.recording.inputs.size() > 0 ? "true" : "false") << "\n"
            << "    }";
    }

    json << "\n  ]\n}";

     
    return createJsonResponse(200, json.str());
}

std::string handlePackRecordingRequest(
    const std::string& authToken,
    const std::string& packId,
    const std::filesystem::path& dataFolder,
    const std::shared_ptr<std::unordered_map<std::string, CustomTrainingData>>& trainingDataPtr) {

    std::string cleanPackId = packId;
    if (!cleanPackId.empty() && (cleanPackId[0] == '/' || cleanPackId[0] == '\\')) {
        cleanPackId = cleanPackId.substr(1);
    }

     

     
    for (const auto& [key, value] : *trainingDataPtr) {
         
    }

    if (authToken != AUTH_TOKEN) {
        return createJsonResponse(401, "{\n\"error\": \"Unauthorized\"\n}");
    }

    bool found = false;
    const CustomTrainingData* foundPack = nullptr;
    std::string foundKey;
    
    //this probably is overkill, remove later
    for (const auto& [key, value] : *trainingDataPtr) {
        // Normalize string
        std::string normalizedCleanPackId = cleanPackId;
        std::string normalizedCode = value.code;

        std::transform(normalizedCleanPackId.begin(), normalizedCleanPackId.end(),
            normalizedCleanPackId.begin(), ::tolower);
        std::transform(normalizedCode.begin(), normalizedCode.end(),
            normalizedCode.begin(), ::tolower);

        normalizedCleanPackId.erase(0, normalizedCleanPackId.find_first_not_of(" \t\r\n"));
        normalizedCleanPackId.erase(normalizedCleanPackId.find_last_not_of(" \t\r\n") + 1);
        normalizedCode.erase(0, normalizedCode.find_first_not_of(" \t\r\n"));
        normalizedCode.erase(normalizedCode.find_last_not_of(" \t\r\n") + 1);

        if (normalizedCleanPackId == normalizedCode) {
            foundPack = &value;
            foundKey = key;
            found = true;
             
            break;
        }

        std::string normalizedKey = key;
        std::transform(normalizedKey.begin(), normalizedKey.end(),
            normalizedKey.begin(), ::tolower);
        normalizedKey.erase(0, normalizedKey.find_first_not_of(" \t\r\n"));
        normalizedKey.erase(normalizedKey.find_last_not_of(" \t\r\n") + 1);

        if (normalizedCleanPackId == normalizedKey) {
            foundPack = &value;
            foundKey = key;
            found = true;
             
            break;
        }
    }
    
    if (!found || !foundPack) {
         
        return createJsonResponse(404, "{\n\"error\": \"Training pack not found\"\n}");
    }

    CustomTrainingData& trainingData = trainingDataPtr->at(foundKey);
    
    std::string sanitizedPackId = cleanPackId;
    for (auto& c : sanitizedPackId) {
        if (c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' || c == '"' || c == '<' || c == '>' || c == '|') {
            c = '_';
        }
    }

    std::filesystem::path packFolder = dataFolder / "TrainingPacks" / sanitizedPackId;
    std::filesystem::path trainingDataFile = packFolder / "trainingpack.txt";
    std::filesystem::path recordingsFile = packFolder / "shots.rec";

    std::string encodedTrainingData;
    std::string shotRecordings;
 

    if (std::filesystem::exists(trainingDataFile)) {
        std::ifstream file(trainingDataFile, std::ios::binary | std::ios::ate);
        if (file) {
            std::streamsize size = file.tellg();
            file.seekg(0, std::ios::beg);
            
            encodedTrainingData.resize(size);
            if (file.read(&encodedTrainingData[0], size)) {
                 
            } else {
                 
                return createJsonResponse(500, "{\n\"error\": \"Failed to read training data\"\n}");
            }
            file.close();
        }
    } else {
         
        return createJsonResponse(404, "{\n\"error\": \"Training pack data not found\"\n}");
    }
    
  
    if (std::filesystem::exists(recordingsFile)) {
        std::ifstream file(recordingsFile, std::ios::binary | std::ios::ate);
        if (file) {
            std::streamsize size = file.tellg();
            file.seekg(0, std::ios::beg);

            shotRecordings.resize(size);
            if (file.read(&shotRecordings[0], size)) {
                 
                for (size_t i = 0; i < shotRecordings.length(); i++) {
                    if (shotRecordings[i] == '\n') {
                        shotRecordings[i] = '.';
                    }
                }
            } else {
                 
                shotRecordings = "";
            }
            file.close();
        }
    } else {
         
    }
    
    std::ostringstream json;
    json << "{\n"
        << "  \"packId\": \"" << cleanPackId << "\",\n"
        << "  \"trainingData\": \"" << encodedTrainingData << "\",\n"
        << "  \"shotsRecording\": \"" << (shotRecordings.empty() ? "" : shotRecordings) << "\"\n"
        << "}";

     
    return createJsonResponse(200, json.str());
}