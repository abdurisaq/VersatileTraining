#include "pch.h"
#include "src/core/VersatileTraining.h"


void VersatileTraining::RenderAllSnapshotsTab() {
   
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f, 0.2f, 0.3f, 0.3f));
    ImGui::BeginChild("FilterBar_AllSnapshots", ImVec2(ImGui::GetContentRegionAvail().x, 80), false); 

    ImGui::TextUnformatted("Search:");
    ImGui::SameLine();
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 6));
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 80);
    ImGui::InputTextWithHint("##SnapshotSearch_All", "Search snapshots...", this->allSnapshotsSearchBuffer, IM_ARRAYSIZE(this->allSnapshotsSearchBuffer));
    ImGui::PopStyleVar();
    ImGui::SameLine();
    if (ImGui::Button("Clear##Search_All", ImVec2(70, 0))) { 
        this->allSnapshotsSearchBuffer[0] = '\0';
    }

    ImGui::Spacing();
    ImGui::TextUnformatted("Filter/Sort by:");
    ImGui::SameLine();
    ImGui::RadioButton("Name##Filter_All", &this->allSnapshotsFilterType, 0); 
    ImGui::SameLine(0, 20);
    ImGui::RadioButton("Date##Filter_All", &this->allSnapshotsFilterType, 1); 
    ImGui::SameLine(0, 20);
    ImGui::RadioButton("Source##Filter_All", &this->allSnapshotsFilterType, 2); 

    ImGui::SameLine(0, 20);
    ImGui::TextUnformatted("Order:");
    ImGui::SameLine();
    if (ImGui::Button(this->allSnapshotsSortAscending ? "^##GallerySort_All" : "v##GallerySort_All", ImVec2(24, 0))) { 
        this->allSnapshotsSortAscending = !this->allSnapshotsSortAscending;
    }
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), this->allSnapshotsSortAscending ? "Ascending" : "Descending");

    if (this->allSnapshotsFilterType == 2) { // Source
        ImGui::SameLine(0, 50);
        ImGui::TextUnformatted("Show:");
        ImGui::SameLine();
        ImGui::RadioButton("All##SourceFilter_All", &this->allSnapshotsSourceFilter, 0); 
        ImGui::SameLine();
        ImGui::RadioButton("Training##SourceFilter_All", &this->allSnapshotsSourceFilter, 1); 
        ImGui::SameLine();
        ImGui::RadioButton("Replay##SourceFilter_All", &this->allSnapshotsSourceFilter, 2);
    }

    ImGui::EndChild(); 
    ImGui::PopStyleColor();

    ImGui::Spacing();

    ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "Saved Snapshots (%zu)", snapshotManager.replayStates.size());
    

    ImGui::Separator();
    ImGui::Spacing();

    ImVec2 galleryAreaSize = ImGui::GetContentRegionAvail();
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.15f, 0.15f, 0.15f, 0.5f));
    ImGui::BeginChild("SnapshotScrollArea_All", galleryAreaSize, true); 

    std::vector<std::pair<size_t, ReplayState>> filteredStates;
    for (size_t i = 0; i < snapshotManager.replayStates.size(); ++i) {
        
        filteredStates.push_back({ i, snapshotManager.replayStates[i] });
    }

    // Sort
    std::sort(filteredStates.begin(), filteredStates.end(),
        [&](const auto& a, const auto& b) {
            const ReplayState& stateA = a.second; 
            const ReplayState& stateB = b.second;
            bool result = false;

            if (this->allSnapshotsFilterType == 0) {  // Name
                std::string nameA = stateA.replayName.empty() ? "Unnamed Snapshot" : stateA.replayName;
                std::string nameB = stateB.replayName.empty() ? "Unnamed Snapshot" : stateB.replayName;
                std::string lowerA = nameA; std::transform(lowerA.begin(), lowerA.end(), lowerA.begin(), ::tolower);
                std::string lowerB = nameB; std::transform(lowerB.begin(), lowerB.end(), lowerB.begin(), ::tolower);
                result = lowerA < lowerB;
            }
            else if (this->allSnapshotsFilterType == 1) {  // Date
                result = stateA.formattedTimeStampOfSaved < stateB.formattedTimeStampOfSaved;
            }
            else if (this->allSnapshotsFilterType == 2) {  // Source
                if (stateA.captureSource == stateB.captureSource) {
                    std::string nameA = stateA.replayName.empty() ? "Unnamed Snapshot" : stateA.replayName;
                    std::string nameB = stateB.replayName.empty() ? "Unnamed Snapshot" : stateB.replayName;
                    result = nameA < nameB;
                }
                else {
                    result = static_cast<int>(stateA.captureSource) < static_cast<int>(stateB.captureSource);
                }
            }
            return this->allSnapshotsSortAscending ? result : !result;
        });

    int displayCount = 0;
    size_t indexToDelete = static_cast<size_t>(-1); 

    for (const auto& pairData : filteredStates) {
        size_t originalIndex = pairData.first; 
        
        const ReplayState& state = snapshotManager.replayStates[originalIndex]; 

        bool showItem = true;
        std::string searchStr = this->allSnapshotsSearchBuffer;
        std::string itemName = state.replayName.empty() ? "Unnamed Snapshot" : state.replayName;
        std::string dateStr = state.formattedTimeStampOfSaved;

        if (this->allSnapshotsFilterType == 2 && this->allSnapshotsSourceFilter > 0) {
            if (this->allSnapshotsSourceFilter == 1 && state.captureSource != CaptureSource::Training) {
                showItem = false;
            }
            else if (this->allSnapshotsSourceFilter == 2 && state.captureSource != CaptureSource::Replay) {
                showItem = false;
            }
        }

        if (showItem && !searchStr.empty()) {
            std::string lowerSearchStr = searchStr;
            std::transform(lowerSearchStr.begin(), lowerSearchStr.end(), lowerSearchStr.begin(), ::tolower);
            std::string lowerItemName = itemName; std::transform(lowerItemName.begin(), lowerItemName.end(), lowerItemName.begin(), ::tolower);
            std::string lowerDateStr = dateStr; std::transform(lowerDateStr.begin(), lowerDateStr.end(), lowerDateStr.begin(), ::tolower);
            // More generic search:
            if (lowerItemName.find(lowerSearchStr) == std::string::npos &&
                lowerDateStr.find(lowerSearchStr) == std::string::npos) {
                showItem = false;
            }
        }

        if (!showItem) {
            continue;
        }

        displayCount++;
        ImGui::PushID(static_cast<int>(originalIndex)); 

        ImGui::PushStyleColor(ImGuiCol_Header, state.captureSource == CaptureSource::Replay ?
            ImVec4(0.15f, 0.2f, 0.3f, 0.9f) : ImVec4(0.2f, 0.3f, 0.15f, 0.9f));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, state.captureSource == CaptureSource::Replay ?
            ImVec4(0.25f, 0.3f, 0.4f, 0.9f) : ImVec4(0.3f, 0.4f, 0.25f, 0.9f));

        std::string sourceIcon = state.captureSource == CaptureSource::Replay ? "[R] " : "[T] "; 
        std::string displayName = sourceIcon + itemName;

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 8));
        bool isOpen = ImGui::CollapsingHeader(displayName.c_str());
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(2);

        ImGui::SameLine(ImGui::GetWindowWidth() - 220);
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", dateStr.c_str());

        if (isOpen) {
            ImGui::Indent(10.0f);
            ImGui::Spacing();

           
            if (state.captureSource == CaptureSource::Replay) {
                ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "Replay Info:");
                ImGui::Indent(10.0f);
                ImGui::Text("Time: %s", state.replayTime.c_str());
                ImGui::Text("Remaining: %s", state.timeRemainingInGame.c_str());
                ImGui::Text("Player: %s", state.focusPlayerName.c_str());
                ImGui::Unindent(10.0f);
                ImGui::Spacing();
            }

            if (ImGui::TreeNode("Car Details")) {
                ImGui::Columns(2, "CarDetailsColumns_All", false); 
                ImGui::SetColumnWidth(0, 180);
                ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "Location:"); ImGui::NextColumn(); ImGui::Text("(%.1f, %.1f, %.1f)", state.carLocation.X, state.carLocation.Y, state.carLocation.Z); ImGui::NextColumn();
                ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "Velocity:"); ImGui::NextColumn(); ImGui::Text("(%.1f, %.1f, %.1f)", state.carVelocity.X, state.carVelocity.Y, state.carVelocity.Z); ImGui::NextColumn();
                ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "Angular Velocity:"); ImGui::NextColumn(); ImGui::Text("(%.1f, %.1f, %.1f)", state.carAngularVelocity.X, state.carAngularVelocity.Y, state.carAngularVelocity.Z); ImGui::NextColumn();
                ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "Rotation:"); ImGui::NextColumn(); ImGui::Text("(Pitch: %d, Yaw: %d, Roll: %d)", state.carRotation.Pitch, state.carRotation.Yaw, state.carRotation.Roll); ImGui::NextColumn();
                ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "Boost:"); ImGui::NextColumn(); ImGui::Text("%d", state.boostAmount); ImGui::NextColumn();
                ImGui::Columns(1);
                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Ball Details")) {
                ImGui::Columns(2, "BallDetailsColumns_All", false); 
                ImGui::SetColumnWidth(0, 180);
                ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "Location:"); ImGui::NextColumn(); ImGui::Text("(%.1f, %.1f, %.1f)", state.ballLocation.X, state.ballLocation.Y, state.ballLocation.Z); ImGui::NextColumn();
                ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "Shot Speed:"); ImGui::NextColumn(); ImGui::Text("%.1f KPH", (state.ballSpeed * 0.036f)); ImGui::NextColumn();
                ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "Shot Direction:"); ImGui::NextColumn(); ImGui::Text("(Pitch: %d, Yaw: %d)", state.ballRotation.Pitch, state.ballRotation.Yaw); ImGui::NextColumn();
                ImGui::Columns(1);
                ImGui::TreePop();
            }
            // End of #selection

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            float buttonWidth = 100.0f;
            float currentSpacing = ImGui::GetStyle().ItemSpacing.x;
            float totalWidth = buttonWidth * 3 + currentSpacing * 2;
            ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - totalWidth) * 0.5f + ImGui::GetCursorPosX());

            bool canLoadSnapshot = isCarRotatable;

            if (canLoadSnapshot) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 0.3f, 1.0f));
                if (ImGui::Button("Load##All", ImVec2(buttonWidth, 30))) {
                    if (originalIndex < snapshotManager.replayStates.size()) {
                        savedReplayState = snapshotManager.replayStates[originalIndex];
                        savedReplayState.ballSet = false;
                        savedReplayState.carLocationSet = false;
                        savedReplayState.carRotationSet = false;
                        
                         
                    }
                }
                ImGui::PopStyleColor(2);
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 0.7f)); 
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.35f, 0.35f, 0.35f, 0.7f)); 
                ImGui::Button("Load##All", ImVec2(buttonWidth, 30)); 
                ImGui::PopStyleColor(2);
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Snapshots can only be loaded when in the Training Editor.");
                }
            }

            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.4f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5f, 0.5f, 0.3f, 1.0f));
            std::string renamePopupID = "RenameSnapshotPopup_All##" + std::to_string(originalIndex);
            if (ImGui::Button("Rename##All", ImVec2(buttonWidth, 30))) { 
                if (originalIndex < snapshotManager.replayStates.size()) { 
                    ImGui::OpenPopup(renamePopupID.c_str());
                }
            }
            ImGui::PopStyleColor(2);

            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.15f, 0.15f, 0.9f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.25f, 0.25f, 0.9f));
            if (ImGui::Button("Delete##All", ImVec2(buttonWidth, 30))) { 
                if (originalIndex < snapshotManager.replayStates.size()) { 
                    indexToDelete = originalIndex; 
                }
            }
            ImGui::PopStyleColor(2);

            if (ImGui::BeginPopupModal(renamePopupID.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "Rename Snapshot: %s",
                    (originalIndex < snapshotManager.replayStates.size() ?
                        (snapshotManager.replayStates[originalIndex].replayName.empty() ? "Unnamed Snapshot" : snapshotManager.replayStates[originalIndex].replayName.c_str())
                        : "Error"));
                ImGui::Separator();

                static char currentRenameBuffer_All[128];
                if (ImGui::IsWindowAppearing()) {
                    if (originalIndex < snapshotManager.replayStates.size()) {
                        strncpy(currentRenameBuffer_All, snapshotManager.replayStates[originalIndex].replayName.c_str(), sizeof(currentRenameBuffer_All) - 1);
                        currentRenameBuffer_All[sizeof(currentRenameBuffer_All) - 1] = '\0';
                    }
                    else {
                        currentRenameBuffer_All[0] = '\0';
                    }
                }

                ImGui::Text("Enter new name:");
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 6));
                ImGui::InputText("##NewSnapshotNameInput_All", currentRenameBuffer_All, IM_ARRAYSIZE(currentRenameBuffer_All)); // Unique ID
                ImGui::PopStyleVar();

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 0.3f, 1.0f));
                if (ImGui::Button("Save##Rename_All", ImVec2(120, 30))) { // Unique ID
                    if (strlen(currentRenameBuffer_All) > 0 && originalIndex < snapshotManager.replayStates.size()) {
                        snapshotManager.replayStates[originalIndex].replayName = currentRenameBuffer_All;
                        storageManager.saveReplayStates(snapshotManager.replayStates, storageManager.saveReplayStateFilePath);
                    }
                    ImGui::CloseCurrentPopup();
                }
                ImGui::PopStyleColor(2);
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.15f, 0.15f, 0.9f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.25f, 0.25f, 0.9f));
                if (ImGui::Button("Cancel##Rename_All", ImVec2(120, 30))) { // Unique ID
                    ImGui::CloseCurrentPopup();
                }
                ImGui::PopStyleColor(2);
                ImGui::EndPopup();
            }
            ImGui::Unindent(10.0f);
        }
        ImGui::PopID();
        ImGui::Separator();

        if (indexToDelete != static_cast<size_t>(-1)) break; 
    }

    if (indexToDelete != static_cast<size_t>(-1)) {
        if (indexToDelete < snapshotManager.replayStates.size()) {
            snapshotManager.replayStates.erase(snapshotManager.replayStates.begin() + indexToDelete);
            UpdateGroupIndicesAfterSnapshotDeletion(indexToDelete); 
            storageManager.saveReplayStates(snapshotManager.replayStates, storageManager.saveReplayStateFilePath);
           
        }
    }


    if (displayCount == 0 && indexToDelete == static_cast<size_t>(-1)) { // Only show if not about to re-render due to delete
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
        ImGui::TextWrapped("No snapshots match your search criteria. Try adjusting your filters or create new snapshots.");
        ImGui::PopStyleColor();
    }

    ImGui::EndChild(); // SnapshotScrollArea_All
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
}