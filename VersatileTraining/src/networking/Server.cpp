// plugin_server.cpp
#include "pch.h"
#include "src/core/VersatileTraining.h"
#include "src/networking/JsonParser.h"
#pragma comment(lib, "ws2_32.lib")

// Configuration constants
const int SERVER_PORT = 7437;
const std::string AUTH_TOKEN = "versatile_training_scanner_token";
const std::string PLAYER_ID_FILE = "player_id.txt";

// Global mutex for thread safety
std::mutex g_mutex;

// Forward declarations for helper functions
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



// Server thread function that can be controlled externally
void VersatileTraining::runServer(
    std::atomic<bool>* isRunning,
    std::string playerId,
    std::shared_ptr<std::unordered_map<std::string, CustomTrainingData>> trainingDataPtr,
    std::filesystem::path dataFolder,
    std::atomic<bool>& hasAction,
    std::string & pendingAction,
    std::mutex& pendingActionMutex) {

    id = playerId;

    // Initialize Winsock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        LOG("WSAStartup failed: {}", result);
        return;
    }

    // Create socket
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        LOG("Error creating socket: {}", WSAGetLastError());
        WSACleanup();
        return;
    }

    // Set socket to non-blocking mode to allow for controlled shutdown
    u_long mode = 1;  // 1 = non-blocking
    if (ioctlsocket(listenSocket, FIONBIO, &mode) == SOCKET_ERROR) {
        LOG("Failed to set socket to non-blocking mode: {}", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return;
    }

    // Setup the TCP listening socket
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(SERVER_PORT);

    // Bind the socket
    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        LOG("Bind failed: {}", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return;
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        LOG("Listen failed: {}", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return;
    }

    LOG("VersatileTraining plugin server started on port {}", SERVER_PORT);

    std::vector<std::thread> clientThreads;

    while (*isRunning) {
        SOCKET clientSocket = accept(listenSocket, NULL, NULL);

        
        if (clientSocket != INVALID_SOCKET) {
            clientThreads.push_back(std::thread(handleClientConnection, clientSocket, trainingDataPtr, dataFolder,std::ref(hasAction), std::ref(pendingAction), std::ref(pendingActionMutex)));
        }
        else {
            if (WSAGetLastError() == WSAEWOULDBLOCK) {
                Sleep(100);  // Sleep for 100ms to avoid busy waiting
            }
            else {
                if (*isRunning) {
                    LOG("Accept failed: {}", WSAGetLastError());
                }
                Sleep(100);
            }
        }

        // Clean up completed threads
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

    LOG("Server shutting down...");
    closesocket(listenSocket);

    for (auto& thread : clientThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    WSACleanup();
    LOG("Server shutdown complete");
}

// Connection handler for individual clients
void handleClientConnection(
    SOCKET clientSocket,
    const std::shared_ptr<std::unordered_map<std::string, CustomTrainingData>>& trainingDataPtr,
    const std::filesystem::path& dataFolder, std::atomic<bool> &hasAction,
    std::string& pendingAction,
    std::mutex& pendingActionMutex) {

    char buffer[327680] = { 0 };
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

    if (bytesReceived > 0) {
        // Null-terminate the received data
        buffer[bytesReceived] = '\0';
        std::string request(buffer);

        std::string method = extractHttpMethod(request);
        std::string endpoint = extractEndpoint(request);
        std::string authToken = extractAuthToken(request);
        std::string response;

        // Log request information
        {
            std::lock_guard<std::mutex> lock(g_mutex);
            LOG("Received {} request for {}", method, endpoint);
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
            // Format: /pack-recording/{packId}
            std::string packId = endpoint.substr(15); // Remove "/pack-recording/"

            // Remove any leading slash from packId
            if (!packId.empty() && (packId[0] == '/' || packId[0] == '\\')) {
                packId = packId.substr(1);
            }

            LOG("Received request for recordings of pack {}", packId);
            response = handlePackRecordingRequest(authToken, packId, dataFolder, trainingDataPtr);
        }
        else {
            // 404 Not Found
            response = createJsonResponse(404, "{\n\"error\": \"Not found\"\n}");
        }

        // Send response
        send(clientSocket, response.c_str(), (int)response.length(), 0);
    }

    // Close the client socket
    closesocket(clientSocket);
}

// Implementation of helper functions
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

    LOG("Sending response: {}", response);
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
        // Return full status with player ID
        LOG("Found auth token, returning player ID: {}", playerId);
        return createJsonResponse(200,
            "{\n"
            "  \"status\": \"plugin_active\",\n"
            "  \"player_id\": \"" + playerId + "\",\n"
            "  \"version\": \"1.0.0\"\n"
            "}");
    }
    else {
        // Return minimal status without player ID
        LOG("No auth token found, returning without player ID");
        return createJsonResponse(200,
            "{\n"
            "  \"status\": \"plugin_active\"\n"
            "}");
    }
}

std::string handleLoadPackRequest(const std::string& authToken, const std::string& body, std::filesystem::path dataFolder, std::atomic<bool>& hasAction,
    std::string& pendingAction,
    std::mutex& pendingActionMutex) {
    LOG("auth token received: {}", authToken);
    LOG("expected token: {}", AUTH_TOKEN);
    if (authToken != AUTH_TOKEN) {
        return createJsonResponse(401,
            "{\n"
            "  \"error\": \"Unauthorized\"\n"
            "}");
    }

    PackInfo pack = parsePack(body);
    
    packInfoToLocalStorage(pack, dataFolder);
    {
        std::lock_guard<std::mutex> lock(pendingActionMutex);
        pendingAction = pack.code;
        hasAction.store(true, std::memory_order_release);
    }
    pack.print();

    //std::replace(body.begin(), body.end(), '.', '\n'); 
    // Return success response
    return createJsonResponse(200,
        "{\n"
        "  \"status\": \"success\",\n"
        "  \"message\": \"Training pack loaded successfully\"\n"
        "}");
}

// New handler to list all available training packs
std::string handleListPacksRequest(
    const std::string& authToken,
    const std::shared_ptr<std::unordered_map<std::string, CustomTrainingData>>& trainingDataPtr) {

    if (authToken != AUTH_TOKEN) {
        return createJsonResponse(401, "{\n\"error\": \"Unauthorized\"\n}");
    }

    if (!trainingDataPtr || trainingDataPtr->empty()) {
        LOG("No training packs available to list");
        return createJsonResponse(200, "{\n\"packs\": []\n}");
    }

    // Construct JSON listing all packs with basic info
    std::ostringstream json;
    json << "{\n  \"packs\": [\n";

    bool firstPack = true;
    for (const auto& [packId, pack] : *trainingDataPtr) {
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

    LOG("Returning list of {} training packs", trainingDataPtr->size());
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
        LOG("Training pack not found: {}", packId);
        return createJsonResponse(404, "{\n\"error\": \"Pack not found\"\n}");
    }

    const CustomTrainingData& pack = it->second;

    // Construct detailed JSON for the pack
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

    LOG("Returning details for pack: {}", packId);
    return createJsonResponse(200, json.str());
}

std::string handlePackRecordingRequest(
    const std::string& authToken,
    const std::string& packId,
    const std::filesystem::path& dataFolder,
    const std::shared_ptr<std::unordered_map<std::string, CustomTrainingData>>& trainingDataPtr) {

    // Remove any leading slash from packId
    std::string cleanPackId = packId;
    if (!cleanPackId.empty() && (cleanPackId[0] == '/' || cleanPackId[0] == '\\')) {
        cleanPackId = cleanPackId.substr(1);
    }

    LOG("Handling pack recording request for pack: {}", cleanPackId);

    if (authToken != AUTH_TOKEN) {
        return createJsonResponse(401, "{\n\"error\": \"Unauthorized\"\n}");
    }

    bool found = false;


    auto it = trainingDataPtr->find(cleanPackId);
    if (it == trainingDataPtr->end()) {
        LOG("Training pack not found in memory: {}", cleanPackId);
        return createJsonResponse(404, "{\n\"error\": \"Training pack not found\"\n}");
    }

    // Get the training data encoded
    CustomTrainingData& trainingData = it->second;
    std::string encodedTrainingData = trainingData.compressAndEncodeTrainingData();

    LOG("Compressed and encoded training data size: {} bytes", encodedTrainingData.length());

    // Sanitize packId to prevent path traversal
    std::string sanitizedPackId = cleanPackId;
    for (auto& c : sanitizedPackId) {
        if (c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' || c == '"' || c == '<' || c == '>' || c == '|') {
            c = '_';
        }
    }

    // Build path to the recordings file for this pack
    std::filesystem::path recordingsFile = dataFolder / "TrainingPacks" / sanitizedPackId / "shots.rec";


    LOG("Looking for recording file at: {}", recordingsFile.string());

    // Get the shot recordings if they exist
    std::string shotRecordings;
    if (std::filesystem::exists(recordingsFile)) {
        std::ifstream file(recordingsFile, std::ios::binary | std::ios::ate);
        if (file) {
            std::streamsize size = file.tellg();
            file.seekg(0, std::ios::beg);

            shotRecordings.resize(size);
            if (file.read(&shotRecordings[0], size)) {
                LOG("Successfully read shot recordings, size: {} bytes", size);
                //removing new line characters to not mess with json output
                for (size_t i = 0; i < shotRecordings.length(); i++) {
                    if (shotRecordings[i] == '\n') {
                        shotRecordings[i] = '.'; //replaceing new line with a dot, which isn't a valid base64 character so know the seperator
                    }
                }

                LOG("Sanitized recording data size: {} bytes", shotRecordings.length());
            }
            else {
                LOG("Failed to read recording file");
                shotRecordings = "";
            }
            file.close();
        }
    }
    else {
        LOG("Recording file not found (this might be normal for packs without recordings)");
    }

    // Create JSON response with both training data and shot recordings
    std::ostringstream json;
    json << "{\n"
        << "  \"packId\": \"" << cleanPackId << "\",\n"
        << "  \"trainingData\": \"" << encodedTrainingData << "\",\n"
        << "  \"shotsRecording\": \"" << (shotRecordings.empty() ? "" : shotRecordings) << "\"\n"
        << "}";

    LOG("Successfully created response with training data and recordings for pack {}", cleanPackId);
    return createJsonResponse(200, json.str());
}