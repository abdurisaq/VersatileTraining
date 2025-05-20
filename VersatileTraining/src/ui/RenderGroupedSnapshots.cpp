#include "pch.h"
#include "src/core/VersatileTraining.h"



void VersatileTraining::RenderGroupedSnapshotsTab() {
    ImGuiStyle& style = ImGui::GetStyle();

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 6));
    float createButtonWidth = ImGui::CalcTextSize("Create New Group").x + style.FramePadding.x * 2 + 10;
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - createButtonWidth - style.ItemSpacing.x);
    ImGui::InputTextWithHint("##GroupSearchInput_Grouped", "Search Groups...", groupSearchBuffer, IM_ARRAYSIZE(groupSearchBuffer));
    ImGui::SameLine();
    if (ImGui::Button("Create New Group##Grouped", ImVec2(createButtonWidth, 0))) {
        newGroupNameInput[0] = '\0';
        ImGui::OpenPopup("CreateSnapshotGroup_Global_Grouped"); // Unique global ID
    }
    ImGui::PopStyleVar();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    RenderCreateGroupPopup(); 

    ImVec2 listRegionSize = ImGui::GetContentRegionAvail();
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.12f, 0.12f, 0.14f, 0.5f));
    ImGui::BeginChild("GroupsScrollArea_Grouped", listRegionSize, true);

    std::string lowerGroupSearch = groupSearchBuffer;
    std::transform(lowerGroupSearch.begin(), lowerGroupSearch.end(), lowerGroupSearch.begin(), ::tolower);

    std::string uidToDeleteAfterLoop = "";

    for (size_t groupIdx = 0; groupIdx < snapshotGroups.size(); ++groupIdx) {
        SnapshotGroup& group = snapshotGroups[groupIdx];

        std::string lowerGroupName = group.name;
        std::transform(lowerGroupName.begin(), lowerGroupName.end(), lowerGroupName.begin(), ::tolower);

        if (!lowerGroupSearch.empty() && lowerGroupName.find(lowerGroupSearch) == std::string::npos) {
            continue;
        }

        ImGui::PushID(group.uid.c_str()); 

        std::string headerText = group.name + " (" + std::to_string(group.snapshotOriginalIndices.size()) + " snapshots)";
        ImVec2 cursorPosBeforeHeader = ImGui::GetCursorPos();

        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.22f, 0.22f, 0.25f, 0.9f));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.28f, 0.28f, 0.32f, 0.9f));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.25f, 0.25f, 0.28f, 0.9f));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 8));

        bool isOpen = ImGui::CollapsingHeader(headerText.c_str(), ImGuiTreeNodeFlags_AllowItemOverlap);

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();

        float addSnapshotsButtonWidth = 100.0f;
        float renameButtonWidth = 80.0f;
        float deleteButtonWidth = 80.0f;
        float buttonGroupWidth = addSnapshotsButtonWidth + style.ItemSpacing.x + renameButtonWidth + style.ItemSpacing.x + deleteButtonWidth;

        float scrollbarWidth = (ImGui::GetScrollMaxY() > 0) ? style.ScrollbarSize : 0.0f;
        float groupStartX = ImGui::GetWindowContentRegionMax().x - buttonGroupWidth - scrollbarWidth;

        ImGui::SetCursorPosX(groupStartX);
        ImGui::SetCursorPosY(cursorPosBeforeHeader.y);

        ImGui::BeginGroup();

        
        std::string addSnapshotsPopupId = "AddSnapshotsToGroup_Popup##" + group.uid;
        if (ImGui::Button("Add Snapshots##GroupedBtn", ImVec2(addSnapshotsButtonWidth, 0))) {
            LOG("Add Snapshots button pressed for group: %s (UID: %s)", group.name.c_str(), group.uid.c_str());
            
            if (snapshotManager.replayStates.empty()) {
                snapshotSelectionForAdding.clear();
            }
            else {
                snapshotSelectionForAdding.assign(snapshotManager.replayStates.size(), false);
            }
            snapshotSearchInAddPopupBuffer[0] = '\0';
            ImGui::OpenPopup(addSnapshotsPopupId.c_str());
        }
        ImGui::SameLine();

        // Rename Button & Popup
        std::string renamePopupId = "RenameSnapshotGroup_Popup##" + group.uid;
        if (ImGui::Button("Rename##GroupedBtn", ImVec2(renameButtonWidth, 0))) {
            LOG("Rename button pressed for group: %s (UID: %s)", group.name.c_str(), group.uid.c_str());
            // groupToRenameUid = group.uid; // Context for the popup
            strncpy(renameGroupNameInput, group.name.c_str(), IM_ARRAYSIZE(renameGroupNameInput) - 1);
            renameGroupNameInput[IM_ARRAYSIZE(renameGroupNameInput) - 1] = '\0';
            ImGui::OpenPopup(renamePopupId.c_str());
        }
        ImGui::SameLine();

        
        std::string deleteGroupPopupId = "ConfirmDeleteGroup_Popup##" + group.uid;
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.15f, 0.15f, 0.9f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.25f, 0.25f, 0.9f));
        if (ImGui::Button("Delete##GroupedBtn", ImVec2(deleteButtonWidth, 0))) {
            ImGui::OpenPopup(deleteGroupPopupId.c_str());
        }
        ImGui::PopStyleColor(2);
        ImGui::EndGroup(); 


        ImVec2 displaySize = ImGui::GetIO().DisplaySize;
        ImVec2 addPopupSize = ImVec2(max(450.0f, displaySize.x * 0.3f), max(500.0f, displaySize.y * 0.6f));
        ImGui::SetNextWindowSize(addPopupSize, ImGuiCond_Appearing);
        if (ImGui::BeginPopupModal(addSnapshotsPopupId.c_str(), NULL, ImGuiWindowFlags_NoScrollbar)) {
            ImGui::Text("Select snapshots to add to group: %s", group.name.c_str()); // Use 'group' directly
            ImGui::InputTextWithHint("##SnapshotSearchInAddPopupInput_InLoop", "Search Snapshots...", snapshotSearchInAddPopupBuffer, IM_ARRAYSIZE(snapshotSearchInAddPopupBuffer));
            ImGui::Separator();

            float listHeight = ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeightWithSpacing() * 2.5f - ImGui::GetStyle().ItemSpacing.y * 3;
            ImGui::BeginChild("SnapshotSelectionListInPopup_InLoop", ImVec2(0, listHeight), true);

            std::string lowerSearch = snapshotSearchInAddPopupBuffer;
            std::transform(lowerSearch.begin(), lowerSearch.end(), lowerSearch.begin(), ::tolower);

            if (snapshotSelectionForAdding.size() != snapshotManager.replayStates.size()) {
                if (snapshotManager.replayStates.empty()) {
                    snapshotSelectionForAdding.clear();
                }
                else {
                    snapshotSelectionForAdding.assign(snapshotManager.replayStates.size(), false);
                }
            }

            for (size_t i = 0; i < snapshotManager.replayStates.size(); ++i) {
                const auto& snapshot = snapshotManager.replayStates[i];
                std::string itemName = snapshot.replayName.empty() ? "Unnamed Snapshot (ID: " + std::to_string(i) + ")" : snapshot.replayName;
                std::string lowerItemName = itemName;
                std::transform(lowerItemName.begin(), lowerItemName.end(), lowerItemName.begin(), ::tolower);

                if (!lowerSearch.empty() && lowerItemName.find(lowerSearch) == std::string::npos) continue;

                bool alreadyInGroup = std::find(group.snapshotOriginalIndices.begin(), group.snapshotOriginalIndices.end(), i) != group.snapshotOriginalIndices.end();

                ImGui::PushID(static_cast<int>(i)); // Unique ID for checkbox within this popup instance
                if (alreadyInGroup) {
                    ImGui::TextDisabled("%s (already in group)", itemName.c_str());
                }
                else {
                    bool tempVal = (i < snapshotSelectionForAdding.size()) ? snapshotSelectionForAdding[i] : false;
                    if (ImGui::Checkbox(itemName.c_str(), &tempVal)) {
                        if (i < snapshotSelectionForAdding.size()) snapshotSelectionForAdding[i] = tempVal;
                    }
                }
                ImGui::PopID();
            }
            ImGui::EndChild();
            ImGui::Separator();

            if (ImGui::Button("Add Selected##Popup_Add_Grouped_InLoop", ImVec2(120, 0))) {
                for (size_t i = 0; i < snapshotSelectionForAdding.size(); ++i) {
                    if (snapshotSelectionForAdding[i]) {
                        if (std::find(group.snapshotOriginalIndices.begin(), group.snapshotOriginalIndices.end(), i) == group.snapshotOriginalIndices.end()) {
                            if (i < snapshotManager.replayStates.size()) {
                                group.snapshotOriginalIndices.push_back(i); // Add to current 'group'
                            }
                        }
                    }
                }
                std::sort(group.snapshotOriginalIndices.begin(), group.snapshotOriginalIndices.end());
                SaveSnapshotGroups();
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel##Popup_Add_Grouped_InLoop", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        // Rename Group Popup Modal
        if (ImGui::BeginPopupModal(renamePopupId.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Enter new name for the group: %s", group.name.c_str()); // Use 'group' for context
            ImGui::PushItemWidth(300);
            ImGui::InputText("##RenameGroupNameInput_InLoop", renameGroupNameInput, IM_ARRAYSIZE(renameGroupNameInput));
            ImGui::PopItemWidth();
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button("Rename Group##Popup_Rename_Grouped_InLoop", ImVec2(120, 0))) {
                if (strlen(renameGroupNameInput) > 0) {
                    group.name = renameGroupNameInput; // Modify current 'group' directly
                    SaveSnapshotGroups();
                    // renameGroupNameInput[0] = '\0'; // Optionally clear shared buffer
                    ImGui::CloseCurrentPopup();
                }
                else {
                    if (gameWrapper) gameWrapper->Toast("Rename Group Error", "New name cannot be empty.", "versatile_training", 5.0f, ToastType_Error);
                    LOG("Rename group error: New name empty.");
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel##Popup_Rename_Grouped_InLoop", ImVec2(120, 0))) {
                // renameGroupNameInput[0] = '\0'; // Optionally clear shared buffer
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        // Delete Group Popup Modal
        if (ImGui::BeginPopupModal(deleteGroupPopupId.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Are you sure you want to delete the group \"%s\"?", group.name.c_str());
            ImGui::TextWrapped("This will not delete the snapshots themselves, only this group.");
            ImGui::Separator();
            if (ImGui::Button("Yes, Delete Group##Confirm_Grouped_InLoop", ImVec2(150, 0))) {
                uidToDeleteAfterLoop = group.uid;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel##Confirm_Grouped_InLoop", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        if (isOpen) {
            ImGui::Indent(10.0f);
            ImGui::Spacing();
            if (group.snapshotOriginalIndices.empty()) {
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "This group is empty. Click 'Add Snapshots' to populate it.");
            }
            else {
                for (size_t i = 0; i < group.snapshotOriginalIndices.size(); ++i) {
                    size_t originalManagerIndex = group.snapshotOriginalIndices[i];
                    if (originalManagerIndex < snapshotManager.replayStates.size()) {
                        RenderSnapshotDetailsInGroup(group, originalManagerIndex, i);
                    }
                    else {
                        ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.0f), "Warning: Snapshot at stored index %zu no longer exists.", originalManagerIndex);
                    }
                    if (i < group.snapshotOriginalIndices.size() - 1) ImGui::Separator();
                }
            }
            ImGui::Unindent(10.0f);
            ImGui::Spacing();
        }
        ImGui::PopID(); // Pop ID for the current group
        if (groupIdx < snapshotGroups.size() - 1) ImGui::Separator();
    } // End of for loop

    if (!uidToDeleteAfterLoop.empty()) {
        snapshotGroups.erase(std::remove_if(snapshotGroups.begin(), snapshotGroups.end(),
            [&](const SnapshotGroup& g_find) { return g_find.uid == uidToDeleteAfterLoop; }), snapshotGroups.end());
        SaveSnapshotGroups();
        uidToDeleteAfterLoop.clear();
    }

    ImGui::EndChild(); // GroupsScrollArea_Grouped
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
}

// RenderSnapshotDetailsInGroup remains largely the same
void VersatileTraining::RenderSnapshotDetailsInGroup(SnapshotGroup& group, size_t snapshotManagerIndex, size_t displayIndexInGroupList) {
    if (snapshotManagerIndex >= snapshotManager.replayStates.size()) {
        ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.0f), "Error: Snapshot data missing for index %zu", snapshotManagerIndex);
        return;
    }
    const ReplayState& state = snapshotManager.replayStates[snapshotManagerIndex];

    std::string uniqueSnapshotID = group.uid + "_snap_" + std::to_string(displayIndexInGroupList); // Keep this unique for nested items
    ImGui::PushID(uniqueSnapshotID.c_str());

    std::string itemName = state.replayName.empty() ? "Unnamed Snapshot (ID: " + std::to_string(snapshotManagerIndex) + ")" : state.replayName;
    std::string dateStr = state.formattedTimeStampOfSaved;
    std::string sourceIcon = state.captureSource == CaptureSource::Replay ? "[R] " : "[T] ";
    std::string displayName = sourceIcon + itemName;

    ImGui::PushStyleColor(ImGuiCol_Header, state.captureSource == CaptureSource::Replay ?
        ImVec4(0.18f, 0.22f, 0.32f, 0.9f) : ImVec4(0.22f, 0.32f, 0.18f, 0.9f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, state.captureSource == CaptureSource::Replay ?
        ImVec4(0.28f, 0.32f, 0.42f, 0.9f) : ImVec4(0.32f, 0.42f, 0.28f, 0.9f));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 7));

    bool snapshotDetailsOpen = ImGui::CollapsingHeader(displayName.c_str()); // This header is fine

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);

    ImGui::SameLine(ImGui::GetWindowWidth() - 280); // Adjust as needed
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", dateStr.c_str());

    if (snapshotDetailsOpen) {
        ImGui::Indent(10.0f);
        ImGui::Spacing();

        if (state.captureSource == CaptureSource::Replay) {
            ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "Replay Info:"); ImGui::Indent();
            ImGui::Text("Time: %s, Player: %s", state.replayTime.c_str(), state.focusPlayerName.c_str()); ImGui::Unindent();
        }
        ImGui::Text("Car Loc: (%.0f, %.0f, %.0f)", state.carLocation.X, state.carLocation.Y, state.carLocation.Z);
        ImGui::Text("Ball Loc: (%.0f, %.0f, %.0f)", state.ballLocation.X, state.ballLocation.Y, state.ballLocation.Z);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        float buttonWidth = 130.0f;
        float availWidth = ImGui::GetContentRegionAvail().x;
        float totalButtonWidth = buttonWidth * 2 + ImGui::GetStyle().ItemSpacing.x;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (availWidth - totalButtonWidth) * 0.5f);

        bool canLoadSnapshot = isCarRotatable;

        if (canLoadSnapshot) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 0.3f, 1.0f));
            if (ImGui::Button("Load Snapshot##DetailsBtn", ImVec2(buttonWidth, 28))) {
                // Action only if enabled (already checked by canLoadSnapshot)
                savedReplayState = state;
                
                savedReplayState.ballSet = false; savedReplayState.carLocationSet = false; savedReplayState.carRotationSet = false;
                
                LOG("Snapshot loaded into savedReplayState for training editor.");
            }
            ImGui::PopStyleColor(2);
        }
        else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 0.7f)); // Greyed out color
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.35f, 0.35f, 0.35f, 0.7f)); // Slightly different hover for disabled
            ImGui::Button("Load Snapshot##DetailsBtn", ImVec2(buttonWidth, 28)); // Click does nothing
            ImGui::PopStyleColor(2);
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Snapshots can only be loaded when in the Training Editor.");
            }
        }

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.4f, 0.1f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.5f, 0.2f, 1.0f));
        if (ImGui::Button("Remove from Group##DetailsBtn", ImVec2(buttonWidth, 28))) {
            auto& indices = group.snapshotOriginalIndices;
            // Make sure to remove the correct element if displayIndexInGroupList is not the actual index in snapshotOriginalIndices
            // Assuming snapshotManagerIndex is the value stored in snapshotOriginalIndices
            indices.erase(std::remove(indices.begin(), indices.end(), snapshotManagerIndex), indices.end());
            SaveSnapshotGroups();
        }
        ImGui::PopStyleColor(2);
        ImGui::Unindent(10.0f);
    }
    ImGui::PopID(); // Pop ID for snapshot details
}

// RenderCreateGroupPopup remains the same as it's global
void VersatileTraining::RenderCreateGroupPopup() {
    if (ImGui::BeginPopupModal("CreateSnapshotGroup_Global_Grouped", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Enter name for the new snapshot group:");
        ImGui::PushItemWidth(300);
        ImGui::InputText("##NewGroupNameInput_Global", newGroupNameInput, IM_ARRAYSIZE(newGroupNameInput));
        ImGui::PopItemWidth();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Create Group##Popup_Create_Global", ImVec2(120, 0))) {
            if (strlen(newGroupNameInput) > 0) {
                SnapshotGroup newGroup;
                newGroup.name = newGroupNameInput;
                newGroup.uid = "group_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
                snapshotGroups.push_back(newGroup);
                SaveSnapshotGroups();
                newGroupNameInput[0] = '\0';
                ImGui::CloseCurrentPopup();
            }
            else {
                if (gameWrapper) gameWrapper->Toast("Group Name Empty", "Group name cannot be empty.", "versatile_training", 5.0f, ToastType_Error);
                LOG("Group name cannot be empty.");
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel##Popup_Create_Global", ImVec2(120, 0))) {
            newGroupNameInput[0] = '\0';
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

// LoadSnapshotGroups, SaveSnapshotGroups, UpdateGroupIndicesAfterSnapshotDeletion remain the same
void VersatileTraining::LoadSnapshotGroups() {
    // Assuming myDataFolderInitialized is a member variable indicating if myDataFolder is set
    // if (!myDataFolderInitialized) { 
    //     LOG("myDataFolder not initialized. Cannot load snapshot groups.");
    //     return;
    // }
    std::filesystem::path groupsFilePath = myDataFolder / "snapshot_groups.txt";
    LOG("Attempting to load snapshot groups from: {}", groupsFilePath.string());

    std::ifstream inFile(groupsFilePath);
    if (!inFile.is_open()) {
        LOG("Failed to open snapshot groups file for reading: {}. No groups loaded or file doesn't exist.", groupsFilePath.string());
        snapshotGroups.clear();
        return;
    }

    snapshotGroups.clear();
    std::string line;
    SnapshotGroup currentGroup;
    int indicesToRead = 0;
    enum class ParseState { UID, NAME, COUNT, INDEX, SEPARATOR };
    ParseState state = ParseState::UID;

    while (std::getline(inFile, line)) {
        if (line.empty() && state != ParseState::SEPARATOR) continue;

        try {
            switch (state) {
            case ParseState::UID:
                if (line == "---END_OF_GROUPS---") {
                    goto end_loading;
                }
                currentGroup = {};
                currentGroup.uid = line;
                state = ParseState::NAME;
                break;
            case ParseState::NAME:
                currentGroup.name = line;
                state = ParseState::COUNT;
                break;
            case ParseState::COUNT:
                indicesToRead = std::stoi(line);
                currentGroup.snapshotOriginalIndices.clear();
                if (indicesToRead > 0) {
                    currentGroup.snapshotOriginalIndices.reserve(indicesToRead);
                }
                if (indicesToRead == 0) {
                    state = ParseState::SEPARATOR;
                }
                else {
                    state = ParseState::INDEX;
                }
                break;
            case ParseState::INDEX:
                currentGroup.snapshotOriginalIndices.push_back(std::stoull(line));
                if (currentGroup.snapshotOriginalIndices.size() >= static_cast<size_t>(indicesToRead)) {
                    state = ParseState::SEPARATOR;
                }
                break;
            case ParseState::SEPARATOR:
                if (line == "---GROUP_SEPARATOR---") {
                    snapshotGroups.push_back(currentGroup);
                    state = ParseState::UID;
                }
                else if (line == "---END_OF_GROUPS---") {
                    if (!currentGroup.uid.empty() &&
                        (indicesToRead == 0 || currentGroup.snapshotOriginalIndices.size() == static_cast<size_t>(indicesToRead))) {
                        snapshotGroups.push_back(currentGroup);
                    }
                    goto end_loading;
                }
                break;
            }
        }
        catch (const std::invalid_argument& ia) {
            LOG("Invalid argument during parsing snapshot groups file: {}. Line: '{}'", ia.what(), line);
            state = ParseState::UID;
            currentGroup = {};
        }
        catch (const std::out_of_range& oor) {
            LOG("Out of range during parsing snapshot groups file: {}. Line: '{}'", oor.what(), line);
            state = ParseState::UID;
            currentGroup = {};
        }
    }

end_loading:
    if (state == ParseState::SEPARATOR && !currentGroup.uid.empty() &&
        (indicesToRead == 0 || currentGroup.snapshotOriginalIndices.size() == static_cast<size_t>(indicesToRead))) {
        bool alreadyAdded = false;
        if (!snapshotGroups.empty() && snapshotGroups.back().uid == currentGroup.uid) {
            alreadyAdded = true;
        }
        if (!alreadyAdded) {
            snapshotGroups.push_back(currentGroup);
        }
    }

    inFile.close();
    LOG("Snapshot groups loaded. Total groups: {}", snapshotGroups.size());
    for (const auto& group_log : snapshotGroups) { // Renamed to avoid conflict with loop var
        LOG("Loaded Group UID: {}, Name: {}, Indices: {}", group_log.uid, group_log.name, group_log.snapshotOriginalIndices.size());
    }
}

void VersatileTraining::SaveSnapshotGroups() {
    // Assuming myDataFolderInitialized is a member variable
    // if (!myDataFolderInitialized) {
    //     LOG("myDataFolder not initialized. Cannot save snapshot groups.");
    //     return;
    // }
    std::filesystem::path groupsFilePath = myDataFolder / "snapshot_groups.txt";
    LOG("Saving snapshot groups to: {}", groupsFilePath.string());

    std::filesystem::create_directories(groupsFilePath.parent_path());
    std::ofstream outFile(groupsFilePath);

    if (!outFile.is_open()) {
        LOG("Failed to open snapshot groups file for writing: {}", groupsFilePath.string());
        return;
    }

    for (size_t i = 0; i < snapshotGroups.size(); ++i) {
        const auto& group_save = snapshotGroups[i]; // Renamed to avoid conflict
        outFile << group_save.uid << "\n";
        outFile << group_save.name << "\n";
        outFile << group_save.snapshotOriginalIndices.size() << "\n";
        for (const auto& index : group_save.snapshotOriginalIndices) {
            outFile << index << "\n";
        }
        if (i < snapshotGroups.size() - 1) {
            outFile << "---GROUP_SEPARATOR---\n";
        }
    }
    outFile << "---END_OF_GROUPS---\n";

    outFile.close();
    LOG("Snapshot groups saved successfully. Total groups: {}", snapshotGroups.size());
}

void VersatileTraining::UpdateGroupIndicesAfterSnapshotDeletion(size_t deletedOriginalIndex) {
    for (auto& group_update : snapshotGroups) { // Renamed to avoid conflict
        auto& indices = group_update.snapshotOriginalIndices;
        for (size_t i = indices.size(); i-- > 0; ) {
            if (indices[i] == deletedOriginalIndex) {
                indices.erase(indices.begin() + i);
            }
            else if (indices[i] > deletedOriginalIndex) {
                indices[i]--;
            }
        }
    }
    SaveSnapshotGroups();
}