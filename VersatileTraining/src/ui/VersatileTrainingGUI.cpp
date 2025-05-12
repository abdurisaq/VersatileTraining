#include "pch.h"
#include "src/core/VersatileTraining.h"

void VersatileTraining::RenderSettings() {
	ImGui::Text("Versatile Training Settings");
    std::string binding = pastBinding;

    ImGui::InputText("##bind_key", &bind_key);
    ImGui::SameLine();
    if (ImGui::Button("Set Bind", ImVec2(80, 0))) {
        cvarManager->removeBind(binding);
        cvarManager->setBind(bind_key, "open_gallery");
        pastBinding = bind_key;
    }
    

	if (ImGui::Button("Find Controller")) {
		HWND hwnd = FindWindowA("LaunchUnrealUWindowsClient", "Rocket League (64-bit, DX11, Cooked)");
		if (!hwnd) {
			LOG("Failed to get Rocket League window handle.");
			return;
		}
		else {
			LOG("Got Rocket League window handle.");
		}

	
	}

	static char trainingCode[20];
	ImGui::InputText("custom Training code",trainingCode, IM_ARRAYSIZE(trainingCode));

	
	ImGui::SliderInt("Boost Amount", &currentShotState.boostAmount, currentTrainingData.boostMin, currentTrainingData.boostMax);
	ImGui::SliderInt("Starting Velocity", &currentShotState.startingVelocity, currentTrainingData.minVelocity, currentTrainingData.maxVelocity);
	
	ImVec2 availableSize = ImGui::GetContentRegionAvail();
	ImVec2 childSize = ImVec2(availableSize.x, availableSize.y);

	ImGui::BeginChild("Training Data", childSize,true);
	for (auto [key, value] : trainingData) {
		if (!value.name.empty()) {
		ImGui::Text("Name: %s", value.name.c_str());
		}
		ImGui::Text("Code: %s", key.c_str());
		ImGui::Text("Boost Amount: %d", value.shots[0].boostAmount);
		ImGui::Text("Starting Velocity: (%d)", value.shots[0].startingVelocity);
	
		if (ImGui::Button(("Edit##" + key).c_str())) {
			
			editingTrainingCode = key;  
			editMode = true;           
		}
		ImGui::SameLine();
		if (ImGui::Button(("Delete##" + key).c_str())) {
			trainingData.erase(key);
		}
		ImGui::Separator();
	}
	ImGui::EndChild();


	if (editMode) {
		ImGui::OpenPopup("Edit Training Pack");

		if (ImGui::BeginPopupModal("Edit Training Pack", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
			CustomTrainingData& data = trainingData[editingTrainingCode];

			ImGui::SliderInt("Boost Amount", &data.shots[0].boostAmount, data.boostMin, data.boostMax);
			ImGui::SliderInt("Starting Velocity", &data.shots[0].startingVelocity, data.minVelocity, data.maxVelocity);
		

			if (ImGui::Button("Save")) {
				
				editMode = false;
			}

			ImGui::EndPopup();

		}

	}

	
}

void VersatileTraining::RenderWindow() {
    if (ImGui::BeginTabBar("SnapshotManagerTabs")) {
        
        if (ImGui::BeginTabItem("Custom Training Packs")) { // Renamed tab
            ImGui::Text("Manage and Load Custom Training Packs");
            ImGui::Separator();

            // --- Load Pack by Code ---
            static char packCodeToLoad[64] = ""; // Buffer for input
            ImGui::InputText("Enter Pack Code", packCodeToLoad, IM_ARRAYSIZE(packCodeToLoad));
            ImGui::SameLine();
            if (ImGui::Button("Load Pack by Code")) {
                if (strlen(packCodeToLoad) > 0) {
                    LOG("Attempting to load pack with code: {}", packCodeToLoad);
                    DownloadTrainingPackById(packCodeToLoad);
                    
                    return;
                }
            }
            ImGui::Separator();

            // --- Search and Filter Controls ---
            static char packSearchBuffer[128] = "";
            static int packSortCriteria = 0; // 0: Name, 1: Code, 2: Num Shots
            static bool packSortAscending = true;

            ImGui::InputText("Search##PackList", packSearchBuffer, IM_ARRAYSIZE(packSearchBuffer));
            ImGui::SameLine();
            if (ImGui::Button("Clear##PackSearch")) { packSearchBuffer[0] = '\0'; }

            ImGui::Text("Sort by:");
            ImGui::RadioButton("Name##PackSort", &packSortCriteria, 0); ImGui::SameLine();
            ImGui::RadioButton("Code##PackSort", &packSortCriteria, 1); ImGui::SameLine();
            ImGui::RadioButton("Num Shots##PackSort", &packSortCriteria, 2);

            ImGui::SameLine();
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, 0)); // Reduce button height
            if (ImGui::Button(packSortAscending ? "Asc ^##PackSortDir" : "Desc v##PackSortDir")) {
                packSortAscending = !packSortAscending;
            }
            ImGui::PopStyleVar();
            ImGui::Separator();

            // --- Pack List ---
            ImGui::Text("Available Packs (%zu):", trainingData.size());

            std::vector<std::pair<std::string, const CustomTrainingData*>> filteredPacksVec;
            for (const auto& pair : trainingData) {
                filteredPacksVec.push_back({ pair.first, &pair.second });
            }

            // Apply text filtering (search)
            if (strlen(packSearchBuffer) > 0) {
                std::string searchStrLower = packSearchBuffer;
                std::transform(searchStrLower.begin(), searchStrLower.end(), searchStrLower.begin(), ::tolower);
                filteredPacksVec.erase(
                    std::remove_if(filteredPacksVec.begin(), filteredPacksVec.end(),
                        [&](const auto& pair) {
                            const CustomTrainingData* pack = pair.second;
                            std::string packNameLower = pack->name.empty() ? pair.first : pack->name;
                            std::transform(packNameLower.begin(), packNameLower.end(), packNameLower.begin(), ::tolower);
                            std::string packCodeLower = pair.first;
                            std::transform(packCodeLower.begin(), packCodeLower.end(), packCodeLower.begin(), ::tolower);

                            return packNameLower.find(searchStrLower) == std::string::npos &&
                                packCodeLower.find(searchStrLower) == std::string::npos;
                        }),
                    filteredPacksVec.end());
            }

            // Apply sorting
            std::sort(filteredPacksVec.begin(), filteredPacksVec.end(),
                [&](const auto& a, const auto& b) {
                    const CustomTrainingData* packA = a.second;
                    const CustomTrainingData* packB = b.second;
                    bool result = false;

                    switch (packSortCriteria) {
                    case 0: { // Name
                        std::string nameA = packA->name.empty() ? a.first : packA->name;
                        std::string nameB = packB->name.empty() ? b.first : packB->name;
                        std::transform(nameA.begin(), nameA.end(), nameA.begin(), ::tolower);
                        std::transform(nameB.begin(), nameB.end(), nameB.begin(), ::tolower);
                        result = nameA < nameB;
                        break;
                    }
                    case 1: { // Code
                        std::string codeA = a.first;
                        std::string codeB = b.first;
                        std::transform(codeA.begin(), codeA.end(), codeA.begin(), ::tolower);
                        std::transform(codeB.begin(), codeB.end(), codeB.begin(), ::tolower);
                        result = codeA < codeB;
                        break;
                    }
                    case 2: // Num Shots
                        result = packA->numShots < packB->numShots;
                        break;
                    }
                    return packSortAscending ? result : !result;
                });

            ImVec2 listRegionSize = ImGui::GetContentRegionAvail();
            ImGui::BeginChild("CustomPacksScrollArea", ImVec2(listRegionSize.x, max(100.0f, listRegionSize.y)), true);

            if (filteredPacksVec.empty()) {
                ImGui::Text("No packs match your criteria or no packs loaded.");
            }
            else {
                for (const auto& pair : filteredPacksVec) {
                    const std::string& packKey = pair.first;
                 
                    if (trainingData.find(packKey) == trainingData.end()) continue; 
                    const CustomTrainingData& packData = trainingData.at(packKey);


                    ImGui::PushID(packKey.c_str());

                    std::string headerName = packData.code.empty() ? (packData.name + " (unpublished)") : (packData.name + " (published)");
                    if (ImGui::CollapsingHeader(headerName.c_str())) {
                        ImGui::Indent();
                        if (!packData.code.empty()) {
                            ImGui::Text("Full Code: %s", packData.code.c_str());
                        }
                        ImGui::Text("Number of Shots: %d", packData.numShots);
                        

                        ImGui::Spacing();

                        if (!packData.code.empty()) {
                            if (ImGui::Button("Play")) {
                                std::string cmd = "load_training " + packData.code;
                                /*cvarManager->executeCommand(cmd);*/
                                gameWrapper->Execute([this, cmd](GameWrapper* gw) {
                                    cvarManager->executeCommand(cmd, false);
                                    });
                            }
                        }

                        ImGui::SameLine();
                        if (ImGui::Button("Delete##Delete")) {
                            ImGui::OpenPopup("DeleteConfirmPopup"); // Generic ID, but context is per pack due to PushID
                        }
                        ImGui::Unindent();

                        if (ImGui::BeginPopupModal("DeleteConfirmPopup", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                            ImGui::Text("Are you sure you want to delete pack: %s?", headerName.c_str());
                            ImGui::Text("This action cannot be undone from the UI.");
                            ImGui::Separator();
                            if (ImGui::Button("Yes, Delete", ImVec2(120, 0))) {
                                std::filesystem::path packFolder;
                                if (!packData.code.empty())
                                {
                                    packFolder = myDataFolder / "TrainingPacks" / packData.code;
                                }
                                else
                                {
									packFolder = myDataFolder / "TrainingPacks" / storageManager.recordingStorage.sanitizeFilename(packData.name);
								}
                                if (!std::filesystem::exists(packFolder)) {
                                    LOG("Training pack folder not found: {}", packFolder.string());
                                    return;
                                }

                                try {
                                    LOG("Deleting training pack folder: {}", packFolder.string());
                                    std::size_t removedCount = std::filesystem::remove_all(packFolder);
                                    LOG("Removed {} files/directories", removedCount);
                                
                                }
                                catch (const std::filesystem::filesystem_error& e) {
                                    LOG("Error deleting training pack folder: {}", e.what());
                                    
                                }
                                trainingData.erase(packKey);
                                if (currentPackKey == packKey) {
                                    currentTrainingData.reset();
                                    currentPackKey.clear();
                                    currentShotState = ShotState();
                                }
                                // TODO: Persist deletion to disk:
                                // storageManager.deletePackFiles(packKey, myDataFolder);
                                // storageManager.saveCompressedTrainingDataWithRecordings(trainingData, myDataFolder); // Then resave the main index
                                LOG("Deleted pack: %s", packKey.c_str());
                                ImGui::CloseCurrentPopup();
                            }
                            ImGui::SameLine();
                            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                                ImGui::CloseCurrentPopup();
                            }
                            ImGui::EndPopup();
                        }
                    }
                    ImGui::PopID();
                    ImGui::Separator();
                }
            }
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        //current
        if (ImGui::BeginTabItem("Current Training Pack")) {
            ImGui::Text("Training Pack: %s", currentTrainingData.name.empty() ? "No pack loaded" : currentTrainingData.name.c_str());

            if (currentTrainingData.shots.size() > 0) {
                ImGui::Text("Total Shots: %d", static_cast<int>(currentTrainingData.shots.size()));
                ImGui::Separator();

                ImVec2 availableSize = ImGui::GetContentRegionAvail();
                ImVec2 childSize = ImVec2(availableSize.x, availableSize.y);
                ImGui::BeginChild("ShotsScrollArea", childSize, true);

                for (size_t i = 0; i < currentTrainingData.shots.size(); ++i) {
                    ImGui::PushID(static_cast<int>(i));

                    char shotName[64];
                    sprintf(shotName, "Shot %d", static_cast<int>(i + 1));

                    // Check if this is the currently edited shot
                    bool isCurrentEditedShot = (i == currentTrainingData.currentEditedShot);
                    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;

                    // Only auto-open the current shot
                    if (!isCurrentEditedShot) {
                        flags = 0;
                    }

                    // Add visual indicator for current shot
                    if (isCurrentEditedShot) {
                        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 220, 0, 255));
                        sprintf(shotName, "Shot %d (Current)", static_cast<int>(i + 1));
                    }

                    if (ImGui::CollapsingHeader(shotName, flags)) {
                        ShotState& shot = currentTrainingData.shots[i];

                        // Car details
                        ImGui::Text("Car Velocity: (%.1f, %.1f, %.1f)", shot.extendedStartingVelocity.X, shot.extendedStartingVelocity.Y, shot.extendedStartingVelocity.Z);
                        ImGui::Text("Boost Amount: %d", shot.boostAmount);
                        ImGui::Text("Starting Velocity: %d", shot.startingVelocity);
                        ImGui::Text("Freeze Car: %s", shot.freezeCar ? "Yes" : "No");

                        // Goal blocker if present
                        if (shot.goalAnchors.first || shot.goalAnchors.second) {
                            ImGui::Separator();
                            ImGui::Text("Goal Blocker: Active");
                            ImGui::Text("Point 1: (%.1f, %.1f, %.1f)", shot.goalBlocker.first.X, shot.goalBlocker.first.Y, shot.goalBlocker.first.Z);
                            ImGui::Text("Point 2: (%.1f, %.1f, %.1f)", shot.goalBlocker.second.X, shot.goalBlocker.second.Y, shot.goalBlocker.second.Z);
                        }

                        // Action buttons for shots
                        if (!isCurrentEditedShot && ImGui::Button("Select This Shot")) {
                            // Load this shot in the editor (you'll need to implement this function)
                            // This would typically set currentTrainingData.currentEditedShot = i
                            // and tell the game to load this shot
                        }
                    }

                    if (isCurrentEditedShot) {
                        ImGui::PopStyleColor();
                    }

                    ImGui::PopID();
                    ImGui::Separator();
                }

                ImGui::EndChild();
            }
            else {
                ImGui::Text("No shots available in this pack.");
            }

            ImGui::EndTabItem();
        }

       

        // Enhanced Snapshot Gallery tab with search, filtering and sorting
        if (ImGui::BeginTabItem("Snapshot Gallery")) {
            static char searchBuffer[128] = "";
            static int filterType = 0; // 0: Name, 1: Date, 2: Source
            static int sourceFilter = 0; // 0: All, 1: Training, 2: Replay
            static bool sortAscending = true; // Toggle for sort direction

            // Search and filter controls
            ImGui::Text("Search and Filter:");
            ImGui::InputText("Search", searchBuffer, IM_ARRAYSIZE(searchBuffer));

            ImGui::SameLine();
            if (ImGui::Button("Clear")) {
                searchBuffer[0] = '\0';
            }

            ImGui::Text("Filter/Sort by:");
            ImGui::RadioButton("Name", &filterType, 0); ImGui::SameLine();
            ImGui::RadioButton("Date", &filterType, 1); ImGui::SameLine();
            ImGui::RadioButton("Source", &filterType, 2);

            ImGui::SameLine();
            if (ImGui::Button(sortAscending ? "^" : "v")) {
                sortAscending = !sortAscending;
            }
            ImGui::SameLine();
            ImGui::Text(sortAscending ? "Ascending" : "Descending");

            if (filterType == 2) {
                ImGui::SameLine();
                ImGui::RadioButton("All", &sourceFilter, 0); ImGui::SameLine();
                ImGui::RadioButton("Training", &sourceFilter, 1); ImGui::SameLine();
                ImGui::RadioButton("Replay", &sourceFilter, 2);
            }

            ImGui::Separator();
            
            // Create a copy of replay states for sorting
            std::vector<std::pair<size_t, ReplayState>> filteredStates;
            for (size_t i = 0; i < snapshotManager.replayStates.size(); ++i) {
                filteredStates.push_back({i, snapshotManager.replayStates[i]});
            }
            
            // Apply sorting based on filter type
            std::sort(filteredStates.begin(), filteredStates.end(), 
                [&](const auto& a, const auto& b) {
                    const ReplayState& stateA = a.second;
                    const ReplayState& stateB = b.second;
                    
                    bool result = false;
                    
                    if (filterType == 0) {  // Sort by name
                        std::string nameA = stateA.replayName.empty() ? "Unnamed Snapshot" : stateA.replayName;
                        std::string nameB = stateB.replayName.empty() ? "Unnamed Snapshot" : stateB.replayName;
                        
                        // Case-insensitive comparison
                        std::string lowerA = nameA;
                        std::transform(lowerA.begin(), lowerA.end(), lowerA.begin(), ::tolower);
                        std::string lowerB = nameB;
                        std::transform(lowerB.begin(), lowerB.end(), lowerB.begin(), ::tolower);
                        
                        result = lowerA < lowerB;
                    }
                    else if (filterType == 1) {  // Sort by date
                        // Parse timestamp and compare
                        // This assumes the timestamp format can be compared lexicographically
                        result = stateA.formattedTimeStampOfSaved < stateB.formattedTimeStampOfSaved;
                    }
                    else if (filterType == 2) {  // Sort by source
                        if (stateA.captureSource == stateB.captureSource) {
                            // If sources are the same, sort by name as secondary key
                            std::string nameA = stateA.replayName.empty() ? "Unnamed Snapshot" : stateA.replayName;
                            std::string nameB = stateB.replayName.empty() ? "Unnamed Snapshot" : stateB.replayName;
                            result = nameA < nameB;
                        } else {
                            result = stateA.captureSource < stateB.captureSource;
                        }
                    }
                    
                    // Reverse for descending order if needed
                    return sortAscending ? result : !result;
                });
            
            ImGui::Text("Saved Snapshots (%d)", snapshotManager.replayStates.size());
            ImGui::Separator();

            ImVec2 availableSize = ImGui::GetContentRegionAvail();
            ImVec2 childSize = ImVec2(availableSize.x, availableSize.y);
            ImGui::BeginChild("SnapshotScrollArea", childSize, true);

            int displayCount = 0;

            // Use the sorted list instead of direct access
            for (const auto& [originalIndex, state] : filteredStates) {
                // Apply filters
                bool showItem = true;
                std::string searchStr = searchBuffer;
                std::string itemName = state.replayName.empty() ? "Unnamed Snapshot" : state.replayName;
                std::string dateStr = state.formattedTimeStampOfSaved;

                // Source filter
                if (filterType == 2 && sourceFilter > 0) {
                    if (sourceFilter == 1 && state.captureSource != CaptureSource::Training) {
                        showItem = false;
                    }
                    else if (sourceFilter == 2 && state.captureSource != CaptureSource::Replay) {
                        showItem = false;
                    }
                }

                // Text search
                if (showItem && searchStr.length() > 0) {
                    std::transform(searchStr.begin(), searchStr.end(), searchStr.begin(), ::tolower);

                    if (filterType == 0) {  // Name filter
                        std::string lowerName = itemName;
                        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
                        if (lowerName.find(searchStr) == std::string::npos) {
                            showItem = false;
                        }
                    }
                    else if (filterType == 1) {  // Date filter
                        std::string lowerDate = dateStr;
                        std::transform(lowerDate.begin(), lowerDate.end(), lowerDate.begin(), ::tolower);
                        if (lowerDate.find(searchStr) == std::string::npos) {
                            showItem = false;
                        }
                    }
                }

                if (!showItem) {
                    continue;
                }

                displayCount++;
                ImGui::PushID(static_cast<int>(originalIndex));

                // Add source indicator to the name
                std::string sourceIndicator = state.captureSource == CaptureSource::Replay ? "[Replay] " : "[Training] ";
                std::string displayName = sourceIndicator + (state.replayName.empty() ? "Unnamed Snapshot" : state.replayName);

                if (ImGui::CollapsingHeader(displayName.c_str())) {
                    // Your existing snapshot display code
                    ImGui::Text("Saved At: %s", state.formattedTimeStampOfSaved.c_str());

                    if (state.captureSource == CaptureSource::Replay) {
                        ImGui::Text("Replay Time: %s", state.replayTime.c_str());
                        ImGui::Text("Time Remaining: %s", state.timeRemainingInGame.c_str());
                        ImGui::Text("Player: %s", state.focusPlayerName.c_str());
                    }

                    ImGui::Text("Boost: %d", state.boostAmount);

                    // Keep your existing tree nodes and button code
                    if (ImGui::TreeNode("Car Details")) {
                        ImGui::Text("Location: (%.1f, %.1f, %.1f)", state.carLocation.X, state.carLocation.Y, state.carLocation.Z);
                        ImGui::Text("Velocity: (%.1f, %.1f, %.1f)", state.carVelocity.X, state.carVelocity.Y, state.carVelocity.Z);
                        ImGui::Text("Angular Velocity: (%.1f, %.1f, %.1f)", state.carAngularVelocity.X, state.carAngularVelocity.Y, state.carAngularVelocity.Z);
                        ImGui::Text("Rotation: (Pitch: %d, Yaw: %d, Roll: %d)", state.carRotation.Pitch, state.carRotation.Yaw, state.carRotation.Roll);
                        ImGui::TreePop();
                    }

                    if (ImGui::TreeNode("Ball Details")) {
                        ImGui::Text("Location: (%.1f, %.1f, %.1f)", state.ballLocation.X, state.ballLocation.Y, state.ballLocation.Z);
                        ImGui::Text("Shot Speed: %.1f", state.ballSpeed);
                        ImGui::Text("Shot Direction: (Pitch: %d, Yaw: %d)", state.ballRotation.Pitch, state.ballRotation.Yaw);
                        ImGui::TreePop();
                    }

                    if (ImGui::Button("Load")) {
                        savedReplayState = state;
                        savedReplayState.ballSet = false;
                        savedReplayState.carLocationSet = false;
                        savedReplayState.carRotationSet = false;
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Delete")) {
                        snapshotManager.replayStates.erase(snapshotManager.replayStates.begin() + originalIndex);
                        ImGui::PopID();
                        continue;
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Rename")) {
                        ImGui::OpenPopup("Rename Snapshot");
                    }

                    // Rename popup
                    if (ImGui::BeginPopupModal("Rename Snapshot", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                        static char newName[128] = "";
                        ImGui::InputText("New Name", newName, IM_ARRAYSIZE(newName));
                        
                        if (ImGui::Button("Save")) {
                            if (strlen(newName) > 0) {
                                // Since we're using the sorted view, we need to use the original index
                                snapshotManager.replayStates[originalIndex].replayName = newName;
                                newName[0] = '\0';
                            }
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Cancel")) {
                            newName[0] = '\0';
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::EndPopup();
                    }
                }

                ImGui::PopID();
                ImGui::Separator();
            }

            if (displayCount == 0) {
                ImGui::Text("No snapshots match your search criteria.");
            }

            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}


