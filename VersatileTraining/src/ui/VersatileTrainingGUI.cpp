#include "pch.h"
#include "src/core/VersatileTraining.h"

void VersatileTraining::displaySpecialKeybind(const std::string& label, int& keyCode) {
    ImGui::AlignTextToFramePadding();
    ImGui::Text("%s", label.c_str());
    ImGui::SameLine(ImGui::GetWindowWidth() * 0.5f);

    // Show the current key
    std::string keyName = getKeyName(keyCode);

    // Create a unique ID for this binding
    std::string bindId = "##specialbind_" + label;

    // Static maps to track binding state individually per binding
    static std::unordered_map<std::string, bool> isBindingMap;
    static std::unordered_map<std::string, int*> currentBindTargetMap;
    static std::unordered_map<std::string, bool> keysWereReleasedMap;

    // Initialize if not present
    if (isBindingMap.find(label) == isBindingMap.end()) {
        isBindingMap[label] = false;
        currentBindTargetMap[label] = nullptr;
        keysWereReleasedMap[label] = false;
    }

    // Button to start binding
    std::string buttonText = keyName + bindId;
    if (ImGui::Button(buttonText.c_str(), ImVec2(80, 0))) {
        isBindingMap[label] = true;
        currentBindTargetMap[label] = &keyCode;
        keysWereReleasedMap[label] = false;

        // Clear any currently pressed keys
        for (int i = 0x08; i <= 0xFE; i++) {
            GetAsyncKeyState(i);
        }
    }

    // If we're binding this key
    if (isBindingMap[label] && currentBindTargetMap[label] == &keyCode) {
        ImGui::SameLine();
        ImGui::Text("Press any key...");

        // First wait for all keys to be released
        bool anyKeyPressed = false;
        for (int i = 0x08; i <= 0xFE; i++) {
            if (GetAsyncKeyState(i) & 0x8000) {
                anyKeyPressed = true;
                break;
            }
        }

        if (!anyKeyPressed) {
            keysWereReleasedMap[label] = true;
        }

        // Only after all keys are released, look for a new key press
        if (keysWereReleasedMap[label]) {
            for (int i = 0x08; i <= 0xFE; i++) {
                if (GetAsyncKeyState(i) & 0x8000) {
                    *(currentBindTargetMap[label]) = i;
                    isBindingMap[label] = false;
                    currentBindTargetMap[label] = nullptr;
                    keysWereReleasedMap[label] = false;
                    // Update currentBindings for display
                    currentBindings[label] = getKeyName(i);
                    // Save the changes
                    storageManager.saveSpecialKeybinds(specialKeybinds, myDataFolder);
                    break;
                }
            }
        }

        // Allow canceling with Escape
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            isBindingMap[label] = false;
            currentBindTargetMap[label] = nullptr;
            keysWereReleasedMap[label] = false;
        }
    }

    ImGui::SameLine();
    if (ImGui::Button(("Reset##" + label).c_str(), ImVec2(50, 0))) {
        // Reset to default value based on the label
        if (label == "Roll Left") keyCode = 'Q';
        else if (label == "Roll Right") keyCode = 'E';
        else if (label == "Decrease Boost") keyCode = '1';
        else if (label == "Increase Boost") keyCode = '2';
        else if (label == "Decrease Velocity") keyCode = '3';
        else if (label == "Increase Velocity") keyCode = '4';

        // Update currentBindings for display
        currentBindings[label] = getKeyName(keyCode);
        storageManager.saveSpecialKeybinds(specialKeybinds, myDataFolder);
    }
}


void VersatileTraining::RenderSettings() {
    ImGui::Text("Versatile Training Settings");

    ImGui::Spacing();

    // Custom button style for plugin interface binding
    ImGui::Text("Open Plugin Interface:");
    ImGui::SameLine();

    auto currentBind = currentBindings.find("open_gallery");
    std::string displayKey = (currentBind != currentBindings.end()) ? currentBind->second : "Unbound";

    static bool isBindingInterfaceKey = false;
    std::string buttonLabel = displayKey + "##InterfaceKey";
    if (ImGui::Button(buttonLabel.c_str(), ImVec2(80, 0))) {
        isBindingInterfaceKey = true;
    }

    if (isBindingInterfaceKey) {
        ImGui::SameLine();
        ImGui::Text("Press any key...");

        for (int i = 0x08; i <= 0xFE; i++) {
            if (GetAsyncKeyState(i) & 0x8000) {
                std::string newBind = getKeyName(i);
                if (!newBind.empty()) {
                    cvarManager->removeBind(pastBinding);
                    cvarManager->setBind(newBind, "open_gallery");
                    pastBinding = newBind;
                    currentBindings["open_gallery"] = newBind;
                    LOG("UI bind set to: {}", newBind);
                }
                isBindingInterfaceKey = false;
                break;
            }
        }

        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            isBindingInterfaceKey = false;
        }
    }

    // Universal keybind display function that uses button approach
    auto DisplayKeybind = [this](const std::string& label, const std::string& command) {
        ImGui::AlignTextToFramePadding();
        ImGui::Text("%s", label.c_str());
        ImGui::SameLine(ImGui::GetWindowWidth() * 0.5f);

        // Show the current key
        std::string currentBind = "";
        auto it = currentBindings.find(command);
        if (it != currentBindings.end()) {
            currentBind = it->second;
        }

        // Create a unique ID for this binding
        std::string bindId = "##bind_" + command;

        // Static variables to track binding state
        static bool isBinding = false;
        static std::string currentBindCommand = "";

        // Button to start binding - Fixed: Create proper buttonLabel
        std::string buttonLabel = (currentBind.empty() ? "Unbound" : currentBind) + bindId;
        if (ImGui::Button(buttonLabel.c_str(), ImVec2(80, 0))) {
            isBinding = true;
            currentBindCommand = command;
        }

        // If we're binding this key
        if (isBinding && currentBindCommand == command) {
            ImGui::SameLine();
            ImGui::Text("Press any key...");

            // Check for key presses
            for (int i = 0x08; i <= 0xFE; i++) { // Check all common keys
                if (GetAsyncKeyState(i) & 0x8000) { // Key just pressed
                    // Remove old binding if it exists
                    if (!currentBind.empty()) {
                        cvarManager->removeBind(currentBind);
                        LOG("Removed old bind: {} -> {}", currentBind, command);
                    }

                    // Set new binding
                    std::string newBind = getKeyName(i);
                    if (!newBind.empty()) {
                        cvarManager->setBind(newBind, command);
                        currentBindings[command] = newBind;
                        LOG("Set new bind: {} -> {}", newBind, command);
                    }

                    isBinding = false;
                    currentBindCommand = "";
                    break;
                }
            }

            // Allow canceling with Escape
            if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
                isBinding = false;
                currentBindCommand = "";
            }
        }

        ImGui::SameLine();
        if (ImGui::Button(("Clear##" + command).c_str(), ImVec2(50, 0))) {
            if (!currentBind.empty()) {
                cvarManager->removeBind(currentBind);
                LOG("Removed bind: {} -> {}", currentBind, command);
                currentBindings.erase(command);
            }
        }
    };

    if (ImGui::BeginTabBar("SettingsTabs")) {

            if (ImGui::BeginTabItem("Pack Overrides")) {
                ImGui::TextWrapped("Define custom gameplay overrides for specific training pack codes. These settings will apply to all shots in the specified pack when active.");
                ImGui::Separator();

                static char currentPackCodeInput[64] = "";
                if (!storageManager.currentEditingOverridePackCode.empty() && strlen(currentPackCodeInput) == 0) {
                    strncpy(currentPackCodeInput, storageManager.currentEditingOverridePackCode.c_str(), sizeof(currentPackCodeInput) - 1);
                    currentPackCodeInput[sizeof(currentPackCodeInput) - 1] = '\0';
                }

                ImGui::InputText("Training Pack Code", currentPackCodeInput, IM_ARRAYSIZE(currentPackCodeInput));
                ImGui::SameLine();
                if (ImGui::Button("Configure##Overrides")) {
                    if (strlen(currentPackCodeInput) > 0) {
                        storageManager.currentEditingOverridePackCode = currentPackCodeInput;
                        if (storageManager.packOverrideSettings.find(storageManager.currentEditingOverridePackCode) == storageManager.packOverrideSettings.end()) {
                            storageManager.packOverrideSettings[storageManager.currentEditingOverridePackCode] = PackOverrideSettings();
                        }
                    }
                    else {
                        storageManager.currentEditingOverridePackCode.clear();
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Clear Input##OverridesClear")) {
                    currentPackCodeInput[0] = '\0';
                    storageManager.currentEditingOverridePackCode.clear();
                }


                if (!storageManager.currentEditingOverridePackCode.empty()) {
                    ImGui::Separator();
                    ImGui::Text("Customizing overrides for: %s", storageManager.currentEditingOverridePackCode.c_str());
                    ImGui::Spacing();

                    PackOverrideSettings& currentSettings = storageManager.packOverrideSettings[storageManager.currentEditingOverridePackCode];

                    auto RenderOverrideSettingCheckbox = [&](const char* label, bool& overrideFlag) {
                        ImGui::Checkbox(label, &overrideFlag);
                        };

                    auto RenderOverrideSettingInt = [&](const char* label, bool& overrideFlag, int& value, int min_val, int max_val, const char* display_format = "%d") {
                        ImGui::Checkbox(label, &overrideFlag);
                        if (overrideFlag) {
                            ImGui::SameLine(); ImGui::PushItemWidth(150);
                            ImGui::SliderInt((std::string("##") + label).c_str(), &value, min_val, max_val, display_format);
                            ImGui::PopItemWidth();
                        }
                        };

                    auto RenderOverrideSettingInputInt = [&](const char* label, bool& overrideFlag, int& value) {
                        ImGui::Checkbox(label, &overrideFlag);
                        if (overrideFlag) {
                            ImGui::SameLine(); ImGui::PushItemWidth(100);
                            ImGui::InputInt((std::string("##") + label).c_str(), &value);
                            ImGui::PopItemWidth();
                        }
                        };

                    auto RenderOverrideSettingRangeInt = [&](const char* label, bool& overrideFlag, int values[2], int min_val, int max_val, const char* display_format = "(%d, %d)") {
                        ImGui::Checkbox(label, &overrideFlag);
                        if (overrideFlag) {
                            ImGui::SameLine(); ImGui::PushItemWidth(200);
                            ImGui::RangeSliderInt((std::string("##") + label).c_str(), &values[0], &values[1], min_val, max_val, display_format);
                            ImGui::PopItemWidth();
                        }
                        };

                    auto RenderOverrideSettingRangeFloat = [&](const char* label, bool& overrideFlag, float values[2], float min_val, float max_val, const char* display_format = "(%.3f, %.3f)") {
                        ImGui::Checkbox(label, &overrideFlag);
                        if (overrideFlag) {
                            ImGui::SameLine(); ImGui::PushItemWidth(200);
                            ImGui::RangeSliderFloat((std::string("##") + label).c_str(), &values[0], &values[1], min_val, max_val, display_format);
                            ImGui::PopItemWidth();
                        }
                        };

                    // Settings based on the image:
                    RenderOverrideSettingInputInt("Boost Limit (-1 unlimited)", currentSettings.overrideBoostLimit, currentSettings.boostLimit);
                    RenderOverrideSettingRangeInt("Player Start Velocity", currentSettings.overridePlayerStartVelocity, currentSettings.playerStartVelocity, 0, 2300, "(%d, %d) uu/s");
                    RenderOverrideSettingRangeFloat("Ball Speed Variance %%", currentSettings.overrideBallSpeedVariancePct, currentSettings.ballSpeedVariancePct, -100.0f, 100.0f, "(%.1f, %.1f)%%");
                    RenderOverrideSettingRangeFloat("Trajectory Rotation Variance %%", currentSettings.overrideTrajectoryRotationVariancePct, currentSettings.trajectoryRotationVariancePct, -100.0f, 100.0f, "(%.1f, %.1f)%%");
                    RenderOverrideSettingRangeFloat("Ball Horiz. Location Variance", currentSettings.overrideBallLocationVarianceXY, currentSettings.ballLocationVarianceXY, -1500.0f, 1500.0f, "(%.0f, %.0f) uu");
                    RenderOverrideSettingRangeFloat("Ball Height Location Variance", currentSettings.overrideBallLocationVarianceHeight, currentSettings.ballLocationVarianceHeight, -1500.0f, 1500.0f, "(%.0f, %.0f) uu");
                    RenderOverrideSettingRangeFloat("Ball Spin", currentSettings.overrideBallSpin, currentSettings.ballSpin, -10.0f, 10.0f, "(%.1f, %.1f) rad/s");
                    RenderOverrideSettingInputInt("Training Time Limit (0 default)", currentSettings.overrideTrainingTimeLimit, currentSettings.trainingTimeLimit);
                    RenderOverrideSettingRangeFloat("Car Horiz. Location Variance", currentSettings.overrideCarLocationVarianceXY, currentSettings.carLocationVarianceXY, -1000.0f, 1000.0f, "(%.0f, %.0f) uu");
                    RenderOverrideSettingRangeFloat("Car Rotation Variance %%", currentSettings.overrideCarRotationVariancePct, currentSettings.carRotationVariancePct, -100.0f, 100.0f, "(%.1f, %.1f)%%");

                    ImGui::Spacing();
                    ImGui::Separator();
                    if (ImGui::Button("Save Overrides##SavePackOverrides")) {
                        storageManager.packOverrideSettings[storageManager.currentEditingOverridePackCode] = currentSettings;
                        currentPackCodeInput[0] = '\0';
                        storageManager.currentEditingOverridePackCode.clear();
                        
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Clear These Overrides##ClearPackOverrides")) {
                        currentSettings = PackOverrideSettings();
                        currentPackCodeInput[0] = '\0';
                        storageManager.currentEditingOverridePackCode.clear();
                    }
                }

                ImGui::Separator();
                ImGui::Text("Packs with Custom Overrides:");
                float listHeight = ImGui::GetTextLineHeightWithSpacing() * 7; // Show ~7 items
                ImGui::BeginChild("OverrideList", ImVec2(0, listHeight), true);
                std::string codeToSelectOnClick;
                for (auto const& [code, settings] : storageManager.packOverrideSettings) {
                    if (settings.HasAnyOverride()) { // Only list packs that actually have an override set
                        if (ImGui::Selectable(code.c_str(), storageManager.currentEditingOverridePackCode == code)) {
                            codeToSelectOnClick = code;
                        }
                    }
                }
                if (!codeToSelectOnClick.empty()) {
                    strncpy(currentPackCodeInput, codeToSelectOnClick.c_str(), sizeof(currentPackCodeInput) - 1);
                    currentPackCodeInput[sizeof(currentPackCodeInput) - 1] = '\0';
                    storageManager.currentEditingOverridePackCode = codeToSelectOnClick;
                    // Ensure entry exists (should already, but good practice)
                    if (storageManager.packOverrideSettings.find(storageManager.currentEditingOverridePackCode) == storageManager.packOverrideSettings.end()) {
                        storageManager.packOverrideSettings[storageManager.currentEditingOverridePackCode] = PackOverrideSettings();
                    }
                }
                ImGui::EndChild();

                ImGui::EndTabItem();
            }
            
        if (ImGui::BeginTabItem("Car Controls")) {
            ImGui::Text("Car Movement & States");
            ImGui::Separator();

            DisplayKeybind("Lock/Unlock Car Rotation", "unlockCar");
            DisplayKeybind("Freeze/Unfreeze Car", "freezeCar");
            DisplayKeybind("Enable/Disable Jump", "removeJump");
            DisplayKeybind("Lock/Unlock Velocity Direction", "lockStartingVelocity");

            ImGui::Spacing();
            ImGui::Text("Car Velocity & Boost (Hold Keys)");
            ImGui::Separator();

            displaySpecialKeybind("Decrease Boost", specialKeybinds.decreaseBoost);
            displaySpecialKeybind("Increase Boost", specialKeybinds.increaseBoost);
            displaySpecialKeybind("Decrease Velocity", specialKeybinds.decreaseVelocity);
            displaySpecialKeybind("Increase Velocity", specialKeybinds.increaseVelocity);

            ImGui::Spacing();
            ImGui::Text("Car Roll Control (Hold Keys)");
            ImGui::Separator();

            displaySpecialKeybind("Roll Left", specialKeybinds.rollLeft);
            displaySpecialKeybind("Roll Right", specialKeybinds.rollRight);

            ImGui::Spacing();
            ImGui::Text("Scene Controls");
            ImGui::Separator();

            DisplayKeybind("Lock/Unlock Scene", "lockScene");

            ImGui::EndTabItem();
        }


        if (ImGui::BeginTabItem("Playback & Recording")) {
            ImGui::Text("Shot Recording Controls");
            ImGui::Separator();

            DisplayKeybind("Start Recording", "startRecording");
            DisplayKeybind("Spawn Bot for Playback", "spawnBot");
            DisplayKeybind("Save Replay Snapshot", "saveReplaySnapshot");
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Goal Blockers")) {
            ImGui::Text("Goal Blocker Controls");
            ImGui::Separator();

            DisplayKeybind("Toggle Goal Blocker Edit Mode", "editGoalBlocker");

            ImGui::Spacing();
            ImGui::Text("Goal Blocker Configuration");
            ImGui::Separator();

            float outlineColorArray[4] = {
                goalBlockerOutlineColor.R / 255.0f,
                goalBlockerOutlineColor.G / 255.0f,
                goalBlockerOutlineColor.B / 255.0f,
                goalBlockerOutlineColor.A / 255.0f
            };

            float gridColorArray[4] = {
                goalBlockerGridColor.R / 255.0f,
                goalBlockerGridColor.G / 255.0f,
                goalBlockerGridColor.B / 255.0f,
                goalBlockerGridColor.A / 255.0f
            };

            if (ImGui::ColorEdit4("Outline Color", outlineColorArray, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar)) {

                goalBlockerOutlineColor.R = outlineColorArray[0] * 255.0f;
                goalBlockerOutlineColor.G = outlineColorArray[1] * 255.0f;
                goalBlockerOutlineColor.B = outlineColorArray[2] * 255.0f;
                goalBlockerOutlineColor.A = outlineColorArray[3] * 255.0f;
            }

            if (ImGui::ColorEdit4("Grid Color", gridColorArray, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar)) {

                goalBlockerGridColor.R = gridColorArray[0] * 255.0f;
                goalBlockerGridColor.G = gridColorArray[1] * 255.0f;
                goalBlockerGridColor.B = gridColorArray[2] * 255.0f;
                goalBlockerGridColor.A = gridColorArray[3] * 255.0f;
            }


            ImGui::SliderInt("Grid Lines", &goalBlockerGridLines, 2, 8, "%d");
            ImGui::SliderFloat("Outline Thickness", &goalBlockerOutlineThickness, 1.0f, 10.0f, "%.1f");
            ImGui::SliderFloat("Grid Thickness", &goalBlockerGridThickness, 0.5f, 5.0f, "%.1f");

            ImGui::EndTabItem();
        }


        if (ImGui::BeginTabItem("Debug & Dev")) {
            ImGui::Text("Development Tools");
            ImGui::Separator();

            DisplayKeybind("Print Current State", "printCurrentState");
            DisplayKeybind("Print Current Shot State", "currentShotState");
            DisplayKeybind("Print Current Pack", "printCurrentPack");
            DisplayKeybind("Print Training Data Map", "printDataMap");
            DisplayKeybind("Find Controller", "findController");

            ImGui::EndTabItem();
        }


        if (ImGui::BeginTabItem("Help & Presets")) {
            ImGui::Text("Key Binding Presets");
            ImGui::Separator();

            if (ImGui::Button("Default Keybinds", ImVec2(150, 0))) {
                ImGui::OpenPopup("DefaultBindConfirm");
            }

            ImGui::SameLine();
            if (ImGui::Button("Clear All Binds", ImVec2(150, 0))) {
                ImGui::OpenPopup("ClearBindConfirm");
            }


            if (ImGui::BeginPopupModal("DefaultBindConfirm", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("Reset all keybinds to default values?");
                ImGui::Text("This will overwrite your current bindings.");
                ImGui::Separator();

                if (ImGui::Button("Yes, Reset to Defaults", ImVec2(180, 0))) {
                    SetDefaultKeybinds();
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }

            if (ImGui::BeginPopupModal("ClearBindConfirm", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("Clear all keybinds?");
                ImGui::Text("You'll need to set them up again manually.");
                ImGui::Separator();

                if (ImGui::Button("Yes, Clear All", ImVec2(150, 0))) {
                    ClearAllKeybinds();
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }

            ImGui::Spacing();
            ImGui::Text("Current Bindings Summary");
            ImGui::Separator();

            ImGui::BeginChild("BindingsList", ImVec2(0, 150), true);
            ImGui::Columns(2, "BindingsColumns");

            ImGui::Text("Command"); ImGui::NextColumn();
            ImGui::Text("Key"); ImGui::NextColumn();
            ImGui::Separator();

            for (const auto& [command, key] : currentBindings) {
                ImGui::Text("%s", command.c_str()); ImGui::NextColumn();
                ImGui::Text("%s", key.c_str()); ImGui::NextColumn();
            }

            ImGui::Columns(1);
            ImGui::EndChild();

            ImGui::Spacing();
            ImGui::Text("Quick Help Guide");
            ImGui::Separator();

            if (ImGui::CollapsingHeader("How to Use Goal Blockers")) {
                ImGui::TextWrapped("1. Enter training editor and press G to enable goal blocker edit mode");
                ImGui::TextWrapped("2. Look at the goal and click to place the first anchor point");
                ImGui::TextWrapped("3. Click again to place the second anchor point");
                ImGui::TextWrapped("4. Press G again to exit edit mode");
            }

            if (ImGui::CollapsingHeader("How to Record and Playback")) {
                ImGui::TextWrapped("1. Setup your shot in the training editor");
                ImGui::TextWrapped("2. Press N to start recording");
                ImGui::TextWrapped("3. Take the shot yourself");
                ImGui::TextWrapped("4. Reset the shot");
                ImGui::TextWrapped("5. Press M to spawn a bot that will play back your inputs");
            }

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}

void VersatileTraining::RenderWindow() {
    if (determiningCodeSync) {
        ImGui::SetWindowSize(ImVec2(0, 0));
        ImGui::OpenPopup("Confirm Code Sync");
    }
    
    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    ImVec2 popupSize = ImVec2(600, 600); 
    ImGui::SetNextWindowPos(ImVec2(displaySize.x * 0.5f - popupSize.x * 0.5f, displaySize.y * 0.5f - 150)); 
    ImGui::SetNextWindowSize(popupSize, ImGuiCond_Appearing);

    if (ImGui::BeginPopupModal("Confirm Code Sync", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
        ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.1f, 1.0f), "Training Pack Code Mismatch Detected!");
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::TextWrapped("The training pack '%s' appears to be published online, but its code ('%s') is not stored locally with the pack.",
            (trainingData && trainingData->count(pendingKey) ? trainingData->at(pendingKey).name.c_str() : "Unknown Pack"),
            pendingCode.c_str());
        ImGui::Spacing();
        ImGui::TextWrapped("Would you like to update the local pack data with this code?");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        float buttonWidth = 120.0f;
        float windowWidth = ImGui::GetWindowSize().x; 
        float spacing = ImGui::GetStyle().ItemSpacing.x;
        float totalButtonWidth = buttonWidth * 2 + spacing;

        ImGui::SetCursorPosX((windowWidth - totalButtonWidth) * 0.5f);
        if (ImGui::Button("Yes, Sync Code", ImVec2(buttonWidth, 0))) {

            (*trainingData)[pendingKey].code = pendingCode;
            CustomTrainingData packData = (*trainingData)[pendingKey];
            trainingData->erase(pendingKey);
            (*trainingData)[pendingCode] = packData;

            LOG("deleting training pack folder: {}", packData.name);
            std::filesystem::path packFolder = myDataFolder / "TrainingPacks" / packData.name;

            if (!std::filesystem::exists(packFolder)) {
                LOG("Training pack folder not found: {}", packFolder.string());
                
            } else {
                try {
                    LOG("Deleting training pack folder: {}", packFolder.string());
                    std::size_t removedCount = std::filesystem::remove_all(packFolder);
                    LOG("Removed {} files/directories", removedCount);
                    
                }
                catch (const std::filesystem::filesystem_error& e) {
                    LOG("Error deleting training pack folder: {}", e.what());
                    
                }
            }
            for (auto& [key, value] : *trainingData) {
                shiftToPositive(value);
            }
            storageManager.saveCompressedTrainingDataWithRecordings(*trainingData, myDataFolder);


            *trainingData = storageManager.loadCompressedTrainingDataWithRecordings(myDataFolder);
            for (auto& [key, value] : *trainingData) {
                shiftToNegative(value);
            }

            
            pendingKey.clear();
            pendingCode.clear();
            determiningCodeSync = false;

            ImGui::CloseCurrentPopup();
            gameWrapper->Execute([this](GameWrapper* gw) {
                std::string cmd = "togglemenu " + GetMenuName();
                cvarManager->executeCommand(cmd, false);
                });
        }

        ImGui::SameLine();
                if (ImGui::Button("No, Later", ImVec2(buttonWidth, 0))) {
            pendingKey.clear();
            pendingCode.clear();
            determiningCodeSync = false;
            ImGui::CloseCurrentPopup();
            
            gameWrapper->Execute([this](GameWrapper* gw) {
                std::string cmd = "togglemenu " + GetMenuName();
                cvarManager->executeCommand(cmd, false);
                });
        }
        ImGui::Spacing();
        ImGui::EndPopup();
        
    }
    if (determiningCodeSync) {
        return; 
    }
    if (ImGui::BeginTabBar("SnapshotManagerTabs")) {
        
        if (ImGui::BeginTabItem("Custom Training Packs")) {
            ImGui::Text("Manage and Load Custom Training Packs");
            ImGui::Separator();

            static char packCodeToLoad[64] = ""; 
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

            static char packSearchBuffer[128] = "";
            static int packSortCriteria = 0; 
            static bool packSortAscending = true;

            ImGui::InputText("Search##PackList", packSearchBuffer, IM_ARRAYSIZE(packSearchBuffer));
            ImGui::SameLine();
            if (ImGui::Button("Clear##PackSearch")) { packSearchBuffer[0] = '\0'; }

            ImGui::Text("Sort by:");
            ImGui::RadioButton("Name##PackSort", &packSortCriteria, 0); ImGui::SameLine();
            ImGui::RadioButton("Code##PackSort", &packSortCriteria, 1); ImGui::SameLine();
            ImGui::RadioButton("Num Shots##PackSort", &packSortCriteria, 2);

            ImGui::SameLine();
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x, 0)); 
            if (ImGui::Button(packSortAscending ? "Asc ^##PackSortDir" : "Desc v##PackSortDir")) {
                packSortAscending = !packSortAscending;
            }
            ImGui::PopStyleVar();
            ImGui::Separator();

            
            ImGui::Text("Available Packs (%zu):", trainingData->size());

            std::vector<std::pair<std::string, const CustomTrainingData*>> filteredPacksVec;
            for (const auto& pair : *trainingData) {
                filteredPacksVec.push_back({ pair.first, &pair.second });
            }

           
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

            
            std::sort(filteredPacksVec.begin(), filteredPacksVec.end(),
                [&](const auto& a, const auto& b) {
                    const CustomTrainingData* packA = a.second;
                    const CustomTrainingData* packB = b.second;
                    bool result = false;

                    switch (packSortCriteria) {
                    case 0: { 
                        std::string nameA = packA->name.empty() ? a.first : packA->name;
                        std::string nameB = packB->name.empty() ? b.first : packB->name;
                        std::transform(nameA.begin(), nameA.end(), nameA.begin(), ::tolower);
                        std::transform(nameB.begin(), nameB.end(), nameB.begin(), ::tolower);
                        result = nameA < nameB;
                        break;
                    }
                    case 1: { 
                        std::string codeA = a.first;
                        std::string codeB = b.first;
                        std::transform(codeA.begin(), codeA.end(), codeA.begin(), ::tolower);
                        std::transform(codeB.begin(), codeB.end(), codeB.begin(), ::tolower);
                        result = codeA < codeB;
                        break;
                    }
                    case 2: 
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
                 
                    if (trainingData->find(packKey) == trainingData->end()) continue; 
                    const CustomTrainingData& packData = trainingData->at(packKey);


                    ImGui::PushID(packKey.c_str());

                    std::string headerNamePart = packData.name;
                    bool isUnpublished = packData.code.empty();
                    std::string headerSuffix = isUnpublished ? " (unpublished)" : " (published)";
                    
                    ImGui::TextUnformatted(headerNamePart.c_str()); 
                    ImGui::SameLine(0.0f, 0.0f); 
                    ImGui::TextUnformatted(headerSuffix.c_str());

                    if (isUnpublished) {
                        ImGui::SameLine();
                        ImGui::TextDisabled("(?)");
                        if (ImGui::IsItemHovered()) {
                            ImGui::BeginTooltip();
                            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                            ImGui::TextUnformatted("Is this actually a published training pack? If so, just load up the training pack once again in the game and you will be prompted to sync the training pack code.");
                            ImGui::PopTextWrapPos();
                            ImGui::EndTooltip();
                        }
                    }
                    
                    std::string collapsingHeaderLabel = "##CollapsingHeader_" + packKey;
                    if (ImGui::CollapsingHeader(collapsingHeaderLabel.c_str())) {
                        ImGui::Indent();
                        if (!packData.code.empty()) {
                            ImGui::Text("Full Code: %s", packData.code.c_str());
                        }
                        ImGui::Text("Number of Shots: %d", packData.numShots);
                        

                        ImGui::Spacing();

                        if (!packData.code.empty()) {
                            if (ImGui::Button("Play")) {
                                std::string cmd = "load_training " + packData.code;
                                
                                gameWrapper->Execute([this, cmd](GameWrapper* gw) {
                                    cvarManager->executeCommand(cmd, false);
                                    });
                            }
                        }

                        ImGui::SameLine();
                        if (ImGui::Button("Delete##Delete")) {
                            ImGui::OpenPopup("DeleteConfirmPopup");
                        }
                        ImGui::Unindent();

                        if (ImGui::BeginPopupModal("DeleteConfirmPopup", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                            ImGui::Text("Are you sure you want to delete pack: %s?", (packData.name + headerSuffix).c_str());
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
                                    
                                } else {
                                    try {
                                        LOG("Deleting training pack folder: {}", packFolder.string());
                                        std::size_t removedCount = std::filesystem::remove_all(packFolder);
                                        LOG("Removed {} files/directories", removedCount);
                                    
                                    }
                                    catch (const std::filesystem::filesystem_error& e) {
                                        LOG("Error deleting training pack folder: {}", e.what());
                                        
                                    }
                                }
                                trainingData->erase(packKey);
                                if (currentPackKey == packKey) {
                                    currentTrainingData.reset();
                                    currentPackKey.clear();
                                    currentShotState = ShotState();
                                }
                               
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

                    bool isCurrentEditedShot = (i == currentTrainingData.currentEditedShot);
                    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;

                    if (!isCurrentEditedShot) {
                        flags = 0;
                    }

                    if (isCurrentEditedShot) {
                        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 220, 0, 255));
                        sprintf(shotName, "Shot %d (Current)", static_cast<int>(i + 1));
                    }

                    if (ImGui::CollapsingHeader(shotName, flags)) {
                        ShotState& shot = currentTrainingData.shots[i];

                        ImGui::Text("Car Velocity: (%.1f, %.1f, %.1f)", shot.extendedStartingVelocity.X, shot.extendedStartingVelocity.Y, shot.extendedStartingVelocity.Z);
                        ImGui::Text("Boost Amount: %d", shot.boostAmount);
                        ImGui::Text("Starting Velocity: %d", shot.startingVelocity);
                        ImGui::Text("Freeze Car: %s", shot.freezeCar ? "Yes" : "No");

                        
                        if (shot.goalAnchors.first || shot.goalAnchors.second) {
                            ImGui::Separator();
                            ImGui::Text("Goal Blocker: Active");
                            ImGui::Text("Point 1: (%.1f, %.1f, %.1f)", shot.goalBlocker.first.X, shot.goalBlocker.first.Y, shot.goalBlocker.first.Z);
                            ImGui::Text("Point 2: (%.1f, %.1f, %.1f)", shot.goalBlocker.second.X, shot.goalBlocker.second.Y, shot.goalBlocker.second.Z);
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

       

        
        if (ImGui::BeginTabItem("Snapshot Gallery")) {
            static char searchBuffer[128] = "";
            static int filterType = 0; 
            static int sourceFilter = 0; 
            static bool sortAscending = true; 

            
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
            
            
            std::vector<std::pair<size_t, ReplayState>> filteredStates;
            for (size_t i = 0; i < snapshotManager.replayStates.size(); ++i) {
                filteredStates.push_back({i, snapshotManager.replayStates[i]});
            }
            
            
            std::sort(filteredStates.begin(), filteredStates.end(), 
                [&](const auto& a, const auto& b) {
                    const ReplayState& stateA = a.second;
                    const ReplayState& stateB = b.second;
                    
                    bool result = false;
                    
                    if (filterType == 0) {  
                        std::string nameA = stateA.replayName.empty() ? "Unnamed Snapshot" : stateA.replayName;
                        std::string nameB = stateB.replayName.empty() ? "Unnamed Snapshot" : stateB.replayName;
                        
                        
                        std::string lowerA = nameA;
                        std::transform(lowerA.begin(), lowerA.end(), lowerA.begin(), ::tolower);
                        std::string lowerB = nameB;
                        std::transform(lowerB.begin(), lowerB.end(), lowerB.begin(), ::tolower);
                        
                        result = lowerA < lowerB;
                    }
                    else if (filterType == 1) {  
                        
                        
                        result = stateA.formattedTimeStampOfSaved < stateB.formattedTimeStampOfSaved;
                    }
                    else if (filterType == 2) {  
                        if (stateA.captureSource == stateB.captureSource) {
                            
                            std::string nameA = stateA.replayName.empty() ? "Unnamed Snapshot" : stateA.replayName;
                            std::string nameB = stateB.replayName.empty() ? "Unnamed Snapshot" : stateB.replayName;
                            result = nameA < nameB;
                        } else {
                            result = stateA.captureSource < stateB.captureSource;
                        }
                    }
                    
                    
                    return sortAscending ? result : !result;
                });
            
            ImGui::Text("Saved Snapshots (%d)", snapshotManager.replayStates.size());
            ImGui::Separator();

            ImVec2 availableSize = ImGui::GetContentRegionAvail();
            ImVec2 childSize = ImVec2(availableSize.x, availableSize.y);
            ImGui::BeginChild("SnapshotScrollArea", childSize, true);

            int displayCount = 0;

            
            for (const auto& [originalIndex, state] : filteredStates) {
                
                bool showItem = true;
                std::string searchStr = searchBuffer;
                std::string itemName = state.replayName.empty() ? "Unnamed Snapshot" : state.replayName;
                std::string dateStr = state.formattedTimeStampOfSaved;

                
                if (filterType == 2 && sourceFilter > 0) {
                    if (sourceFilter == 1 && state.captureSource != CaptureSource::Training) {
                        showItem = false;
                    }
                    else if (sourceFilter == 2 && state.captureSource != CaptureSource::Replay) {
                        showItem = false;
                    }
                }

                
                if (showItem && searchStr.length() > 0) {
                    std::transform(searchStr.begin(), searchStr.end(), searchStr.begin(), ::tolower);

                    if (filterType == 0) {  
                        std::string lowerName = itemName;
                        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
                        if (lowerName.find(searchStr) == std::string::npos) {
                            showItem = false;
                        }
                    }
                    else if (filterType == 1) {  
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

                
                std::string sourceIndicator = state.captureSource == CaptureSource::Replay ? "[Replay] " : "[Training] ";
                std::string displayName = sourceIndicator + (state.replayName.empty() ? "Unnamed Snapshot" : state.replayName);

                if (ImGui::CollapsingHeader(displayName.c_str())) {
                    
                    ImGui::Text("Saved At: %s", state.formattedTimeStampOfSaved.c_str());

                    if (state.captureSource == CaptureSource::Replay) {
                        ImGui::Text("Replay Time: %s", state.replayTime.c_str());
                        ImGui::Text("Time Remaining: %s", state.timeRemainingInGame.c_str());
                        ImGui::Text("Player: %s", state.focusPlayerName.c_str());
                    }

                    ImGui::Text("Boost: %d", state.boostAmount);

                    
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

                    
                    if (ImGui::BeginPopupModal("Rename Snapshot", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                        static char newName[128] = "";
                        ImGui::InputText("New Name", newName, IM_ARRAYSIZE(newName));
                        
                        if (ImGui::Button("Save")) {
                            if (strlen(newName) > 0) {
                                
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

