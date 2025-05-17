#include "pch.h"
#include "src/core/VersatileTraining.h"

void VersatileTraining::displaySpecialKeybind(const std::string& label, int& keyCode) {
    ImGui::AlignTextToFramePadding();
    ImGui::Text("%s", label.c_str());
    ImGui::SameLine(ImGui::GetWindowWidth() * 0.5f);

    std::string keyName = getKeyName(keyCode);

  
    std::string bindId = "##specialbind_" + label;

    
    static std::unordered_map<std::string, bool> isBindingMap;
    static std::unordered_map<std::string, int*> currentBindTargetMap;
    static std::unordered_map<std::string, bool> keysWereReleasedMap;

    if (isBindingMap.find(label) == isBindingMap.end()) {
        isBindingMap[label] = false;
        currentBindTargetMap[label] = nullptr;
        keysWereReleasedMap[label] = false;
    }

   
    std::string buttonText = keyName + bindId;
    if (ImGui::Button(buttonText.c_str(), ImVec2(80, 0))) {
        isBindingMap[label] = true;
        currentBindTargetMap[label] = &keyCode;
        keysWereReleasedMap[label] = false;

        
        for (int i = 0x08; i <= 0xFE; i++) {
            GetAsyncKeyState(i);
        }
    }

    
    if (isBindingMap[label] && currentBindTargetMap[label] == &keyCode) {
        ImGui::SameLine();
        ImGui::Text("Press any key...");

        
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

        
        if (keysWereReleasedMap[label]) {
            for (int i = 0x08; i <= 0xFE; i++) {
                if (GetAsyncKeyState(i) & 0x8000) {
                    *(currentBindTargetMap[label]) = i;
                    isBindingMap[label] = false;
                    currentBindTargetMap[label] = nullptr;
                    keysWereReleasedMap[label] = false;
                    
                    currentBindings[label] = getKeyName(i);
                    
                    storageManager.saveSpecialKeybinds(specialKeybinds, myDataFolder);
                    break;
                }
            }
        }

        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            isBindingMap[label] = false;
            currentBindTargetMap[label] = nullptr;
            keysWereReleasedMap[label] = false;
        }
    }

    ImGui::SameLine();
    if (ImGui::Button(("Reset##" + label).c_str(), ImVec2(50, 0))) {
       
        if (label == "Roll Left") keyCode = 'Q';
        else if (label == "Roll Right") keyCode = 'E';
        else if (label == "Decrease Boost") keyCode = '1';
        else if (label == "Increase Boost") keyCode = '2';
        else if (label == "Decrease Velocity") keyCode = '3';
        else if (label == "Increase Velocity") keyCode = '4';

        
        currentBindings[label] = getKeyName(keyCode);
        storageManager.saveSpecialKeybinds(specialKeybinds, myDataFolder);
    }
}


void VersatileTraining::RenderSettings() {
    ImGui::Text("Versatile Training Settings");

    ImGui::Spacing();

    
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

    
    auto DisplayKeybind = [this](const std::string& label, const std::string& command) {
        ImGui::AlignTextToFramePadding();
        ImGui::Text("%s", label.c_str());
        ImGui::SameLine(ImGui::GetWindowWidth() * 0.5f);

        
        std::string currentBind = "";
        auto it = currentBindings.find(command);
        if (it != currentBindings.end()) {
            currentBind = it->second;
        }

        std::string bindId = "##bind_" + command;

        
        static bool isBinding = false;
        static std::string currentBindCommand = "";

        std::string buttonLabel = (currentBind.empty() ? "Unbound" : currentBind) + bindId;
        if (ImGui::Button(buttonLabel.c_str(), ImVec2(80, 0))) {
            isBinding = true;
            currentBindCommand = command;
        }

        
        if (isBinding && currentBindCommand == command) {
            ImGui::SameLine();
            ImGui::Text("Press any key...");

            
            for (int i = 0x08; i <= 0xFE; i++) { 
                if (GetAsyncKeyState(i) & 0x8000) {
                    
                    if (!currentBind.empty()) {
                        cvarManager->removeBind(currentBind);
                        LOG("Removed old bind: {} -> {}", currentBind, command);
                    }

                    
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
                float listHeight = ImGui::GetTextLineHeightWithSpacing() * 7; 
                ImGui::BeginChild("OverrideList", ImVec2(0, listHeight), true);
                std::string codeToSelectOnClick;
                for (auto const& [code, settings] : storageManager.packOverrideSettings) {
                    if (settings.HasAnyOverride()) { 
                        if (ImGui::Selectable(code.c_str(), storageManager.currentEditingOverridePackCode == code)) {
                            codeToSelectOnClick = code;
                        }
                    }
                }
                if (!codeToSelectOnClick.empty()) {
                    strncpy(currentPackCodeInput, codeToSelectOnClick.c_str(), sizeof(currentPackCodeInput) - 1);
                    currentPackCodeInput[sizeof(currentPackCodeInput) - 1] = '\0';
                    storageManager.currentEditingOverridePackCode = codeToSelectOnClick;
                    
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
            if (ImGui::CollapsingHeader("Online Training Pack Hub")) {
                ImGui::TextWrapped("Share your custom training packs and discover new ones created by the community through the Versatile Training Hub!");
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::TextWrapped("Visit the Hub here:");
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.8f, 1.0f, 1.0f)); // Light blue for link
                ImGui::TextUnformatted("https://versatile-training-hub.vercel.app/");
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Click to copy link to clipboard");
                }
                if (ImGui::IsItemClicked()) {
                    ImGui::SetClipboardText("https://versatile-training-hub.vercel.app/");
                }
                ImGui::PopStyleColor();

                ImGui::Spacing();
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.6f, 1.0f), "Uploading Your Packs:");
                ImGui::BulletText("Make sure your game is running in the background; the website will connect for any uploading/downloading.");
                ImGui::BulletText("On the Hub website, create an account.");
                ImGui::BulletText("Go to 'Upload Pack'.");
                ImGui::BulletText("Select the training pack you want to upload. Note: If you don't see the training pack, ensure it has been published in-game (it needs a code).");
                ImGui::BulletText("Fill in the description and any tags you want.");

                ImGui::Spacing();
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.6f, 1.0f), "Updating Your Packs:");
                ImGui::BulletText("To update your training pack, first make sure to save any local modifications in-game.");
                ImGui::BulletText("Then, head over to the website and go to your profile.");
                ImGui::BulletText("Under the 'Your Training Packs' tab, navigate to the one you want to update and click 'Update from Plugin'. (Your game must be running for this).");

                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.6f, 1.0f, 0.8f, 1.0f), "Downloading & Playing Packs:");
                ImGui::BulletText("Visit the Versatile Training Hub website (your game needs to be running).");
                ImGui::BulletText("Browse or search for packs shared by other users.");
                ImGui::BulletText("Once you find a pack you like, click 'Load in Game'. The plugin will then download and install it for you, and it should load automatically.");

                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "About the Versatile Training Hub:");
                
                ImGui::BulletText("The Hub is a community platform for sharing and discovering training packs.");
                ImGui::BulletText("It allows users to upload their pack data, which are then made available for others to download and play directly in-game via this plugin.");
                ImGui::BulletText("This facilitates a central place for the community to exchange diverse training scenarios, enhancing the utility of the Versatile Training plugin.");

                ImGui::Spacing();
            }
            if (ImGui::CollapsingHeader("How to Use Goal Blockers")) {
                ImGui::TextWrapped("1. Enter training editor and press G to enable goal blocker edit mode");
                ImGui::TextWrapped("2. Look at the goal and click your middle mouse button to place the first anchor point");
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
    
    if (packCodeCopyFlashTimer > 0.0f) {
        packCodeCopyFlashTimer -= ImGui::GetIO().DeltaTime;
        if (packCodeCopyFlashTimer < 0.0f) {
            packCodeCopyFlashTimer = 0.0f;
        }
    }

    ImGuiStyle& style = ImGui::GetStyle();
    float origItemSpacing = style.ItemSpacing.y;
    float origFramePadding = style.FramePadding.y;

    
    style.ItemSpacing.y = 8.0f;
    style.FramePadding.y = 6.0f;

    if (firstTime) {
        ImGui::SetWindowSize(ImVec2(0, 0));
        if (canSpawnWelcomeMessage) {

            ImGui::OpenPopup("Welcome to Versatile Training!");
        }
        
    }

    ImVec2 displaySizeWelcome = ImGui::GetIO().DisplaySize;
    
    ImVec2 welcomePopupSize = ImVec2(max(600.0f, displaySizeWelcome.x * 0.45f), max(550.0f, displaySizeWelcome.y * 0.6f));
    ImGui::SetNextWindowSize(welcomePopupSize, ImGuiCond_Appearing);
    ImGui::SetNextWindowPos(ImVec2(displaySizeWelcome.x * 0.5f - welcomePopupSize.x * 0.5f, displaySizeWelcome.y * 0.5f - welcomePopupSize.y * 0.5f), ImGuiCond_Appearing);

    if (ImGui::BeginPopupModal("Welcome to Versatile Training!", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); 
        ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.1f, 1.0f), "Welcome to Versatile Training!");
        ImGui::PopFont();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::TextWrapped("Thanks for installing Versatile Training! Here's a quick guide to get you started:");
        ImGui::Spacing();

        ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "Core Features:");
        ImGui::BulletText("Custom Training Packs: Create, share, and play advanced training scenarios.");
        ImGui::BulletText("Snapshot System: Save and load game states from replays or training (see 'Snapshot Gallery' tab).");
        ImGui::BulletText("Goal Blocker: Set up custom goal blockers for precision practice (see 'Goal Blockers' tab in the settings).");
        ImGui::BulletText("Recording & Playback: Record your attempts and use them for bot playback (see the 'Playback & Recording' tab in the settings).");
        ImGui::BulletText("Versatile Training Hub: Upload and download training packs from the community (see 'Online Training Pack Hub' in Help).");
        ImGui::BulletText("Pack Overrides: Customize existing training packs, even those by other creators (see 'Pack Overrides' tab in settings).");
        ImGui::Spacing();

        ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "Versatile Training Hub:");
        ImGui::TextWrapped("Discover and share packs with the community at:");
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.8f, 1.0f, 1.0f));
        ImGui::TextUnformatted("https://versatile-training-hub.vercel.app/");
        ImGui::PopStyleColor();
        ImGui::SameLine();
        if (ImGui::SmallButton("Copy Link##Welcome")) {
            ImGui::SetClipboardText("https://versatile-training-hub.vercel.app/");
        }
        ImGui::Spacing();

        ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "Keybinds:");
        ImGui::BulletText("Default keybinds are set for common actions.");
        ImGui::BulletText("View and customize all keybinds in BakkesMod (F2) -> Plugins -> Versatile Training.");
        ImGui::BulletText("The main plugin window can be opened with the key bound to 'Open Plugin Interface' (Default F3).");
        ImGui::Spacing();

        ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "Getting Started:");
        ImGui::BulletText("Try loading a pack from the 'Custom Training Packs' tab or explore the 'Snapshot Gallery'!");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        float buttonWidthWelcome = 120.0f;
        // Center the button
        float windowContentWidth = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;
        ImGui::SetCursorPosX((windowContentWidth - buttonWidthWelcome) * 0.5f);

        if (ImGui::Button("Got it!", ImVec2(buttonWidthWelcome, 30))) {
            firstTime = false; 
            gameWrapper->Execute([this](GameWrapper* gw) {
                std::string cmd = "togglemenu " + GetMenuName();
                cvarManager->executeCommand(cmd, false);
            });
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    if (firstTime) {

        style.ItemSpacing.y = origItemSpacing;
        style.FramePadding.y = origFramePadding;
        return;
    }


    if (determiningCodeSync) {
        ImGui::SetWindowSize(ImVec2(0, 0));
        ImGui::OpenPopup("Confirm Code Sync");
    }

    
    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    ImVec2 popupSize = ImVec2(600, 600);
    ImGui::SetNextWindowPos(ImVec2(displaySize.x * 0.5f - popupSize.x * 0.5f, displaySize.y * 0.5f - 150));
    ImGui::SetNextWindowSize(popupSize, ImGuiCond_Appearing);

    
    if (ImGui::BeginPopupModal("Confirm Code Sync", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
        
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); 
        ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.1f, 1.0f), "Training Pack Code Mismatch Detected!");
        ImGui::PopFont();

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

        
        float buttonWidth = 150.0f;
        float windowWidth = ImGui::GetWindowSize().x;
        float spacing = ImGui::GetStyle().ItemSpacing.x;
        float totalButtonWidth = buttonWidth * 2 + spacing;

        ImGui::SetCursorPosX((windowWidth - totalButtonWidth) * 0.5f);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 0.3f, 1.0f));
        if (ImGui::Button("Yes, Sync Code", ImVec2(buttonWidth, 30))) {
            
            (*trainingData)[pendingKey].code = pendingCode;
            CustomTrainingData packData = (*trainingData)[pendingKey];
            trainingData->erase(pendingKey);
            (*trainingData)[pendingCode] = packData;

            LOG("deleting training pack folder: {}", packData.name);
            std::filesystem::path packFolder = myDataFolder / "TrainingPacks" / packData.name;

            if (!std::filesystem::exists(packFolder)) {
                LOG("Training pack folder not found: {}", packFolder.string());
            }
            else {
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
        ImGui::PopStyleColor(2);

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.3f, 0.3f, 1.0f));
        if (ImGui::Button("No, Later", ImVec2(buttonWidth, 30))) {
            pendingKey.clear();
            pendingCode.clear();
            determiningCodeSync = false;
            ImGui::CloseCurrentPopup();

            gameWrapper->Execute([this](GameWrapper* gw) {
                std::string cmd = "togglemenu " + GetMenuName();
                cvarManager->executeCommand(cmd, false);
                });
        }
        ImGui::PopStyleColor(2);

        ImGui::Spacing();
        ImGui::EndPopup();
    }

    
    if (determiningCodeSync) {
        
        style.ItemSpacing.y = origItemSpacing;
        style.FramePadding.y = origFramePadding;
        return;
    }

    
    if (ImGui::BeginTabBar("MainTabBar", ImGuiTabBarFlags_FittingPolicyResizeDown)) {
        
        if (ImGui::BeginTabItem("Custom Training Packs")) {
            ImGui::Spacing();

            
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 8));
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));

            {
                ImGui::BeginGroup();

                
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 130); 
                static char packCodeToLoad[64] = "";
                ImGui::InputTextWithHint("##PackCodeInput", "Enter Training Pack Code", packCodeToLoad, IM_ARRAYSIZE(packCodeToLoad));
                ImGui::SameLine();

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.8f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.5f, 0.9f, 1.0f));
                if (ImGui::Button("Load Pack", ImVec2(120, 0))) {
                    if (strlen(packCodeToLoad) > 0) {
                        LOG("Attempting to load pack with code: {}", packCodeToLoad);
                        DownloadTrainingPackById(packCodeToLoad);
                    }
                }
                ImGui::PopStyleColor(2);

                ImGui::EndGroup();
            }

            ImGui::PopStyleColor();
            ImGui::PopStyleVar();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            
            {
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 6));

                
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted("Filter:");
                ImGui::SameLine();

                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.45f);
                ImGui::InputTextWithHint("##SearchPacks", "Search by name or code...", packSearchBuffer, IM_ARRAYSIZE(packSearchBuffer));

                ImGui::SameLine();
                if (ImGui::Button("X##ClearSearch", ImVec2(24, 0))) {
                    packSearchBuffer[0] = '\0';
                }

                ImGui::SameLine(0, 20);

                
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted("Sort:");
                ImGui::SameLine();

                ImGui::SetNextItemWidth(120);
                const char* sortOptions[] = { "Name", "Code", "Shots" };
                ImGui::Combo("##SortBy", &packSortCriteria, sortOptions, IM_ARRAYSIZE(sortOptions));

                ImGui::SameLine();
                if (ImGui::Button(packSortAscending ? "^##SortDir" : "v##SortDir", ImVec2(24, 0))) {
                    packSortAscending = !packSortAscending;
                }

                ImGui::PopStyleVar();
            }

            ImGui::Spacing();

           
            ImGui::BeginGroup();
            {
                ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "Available Packs (%zu)", trainingData->size());
                ImGui::SameLine(ImGui::GetWindowWidth() - 40);
                if (ImGui::Button("?##HelpPacks", ImVec2(24, 0))) {
                    ImGui::OpenPopup("TrainingPacksHelp");
                }

                if (ImGui::BeginPopup("TrainingPacksHelp")) {
                    ImGui::TextUnformatted("Training Pack Information");
                    ImGui::Separator();
                    ImGui::TextWrapped("- Published packs have a code and can be played directly");
                    ImGui::TextWrapped("- Unpublished packs need to be loaded in-game to acquire a code");
                    ImGui::TextWrapped("- Click on a pack to expand its details");
                    ImGui::TextWrapped("- Use the search box to filter by name or code");
                    ImGui::EndPopup();
                }
            }
            ImGui::EndGroup();

            ImGui::Spacing();

            
            ImVec2 listRegionSize = ImGui::GetContentRegionAvail();
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.15f, 0.15f, 0.15f, 0.5f));
            ImGui::BeginChild("PacksScrollArea", listRegionSize, true);

            
            std::vector<std::pair<std::string, const CustomTrainingData*>> filteredPacksVec;

            
            for (const auto& [key, value] : *trainingData) {
                if (strlen(packSearchBuffer) > 0) {
                    std::string lowerName = value.name;
                    std::string lowerCode = value.code;
                    std::string lowerSearch = packSearchBuffer;

                    
                    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
                    std::transform(lowerCode.begin(), lowerCode.end(), lowerCode.begin(), ::tolower);
                    std::transform(lowerSearch.begin(), lowerSearch.end(), lowerSearch.begin(), ::tolower);

                    if (lowerName.find(lowerSearch) == std::string::npos &&
                        lowerCode.find(lowerSearch) == std::string::npos) {
                        continue;
                    }
                }

                filteredPacksVec.push_back({ key, &value });
            }

            
            int sortCriteria = packSortCriteria;
            bool sortAscending = packSortAscending;

            std::sort(filteredPacksVec.begin(), filteredPacksVec.end(),
                [sortCriteria, sortAscending](const auto& a, const auto& b) {
                    bool result = false;

                    switch (sortCriteria) {
                    case 0: // Name
                        result = a.second->name < b.second->name;
                        break;
                    case 1: // Code
                        result = a.second->code < b.second->code;
                        break;
                    case 2: // Shots
                        result = a.second->numShots < b.second->numShots;
                        break;
                    default:
                        result = a.first < b.first;
                    }

                    return sortAscending ? result : !result;
                });

            
            if (filteredPacksVec.empty()) {
                ImGui::Spacing();
                ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No packs match your criteria or no packs loaded.");
                ImGui::PopFont();
            }
            else {
                for (const auto& pair : filteredPacksVec) {
                    const std::string& packKey = pair.first;
                    const CustomTrainingData& packData = *pair.second;

                    
                    ImGui::PushID(packKey.c_str());

                    
                    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.2f, 0.2f, 0.9f));
                    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.3f, 0.3f, 0.3f, 0.9f));
                    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.25f, 0.25f, 0.25f, 0.9f));

                    bool isUnpublished = packData.code.empty();
                    std::string statusIcon = isUnpublished ? "! " : "+ ";
                    std::string displayName = statusIcon + packData.name;

                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 8));
                    bool isOpen = ImGui::CollapsingHeader(displayName.c_str(), ImGuiTreeNodeFlags_DefaultOpen);
                    ImGui::PopStyleVar();

                    ImGui::PopStyleColor(3);

                    
                    ImGui::SameLine(ImGui::GetWindowWidth() - 110);
                    if (isUnpublished) {
                        ImGui::TextColored(ImVec4(0.9f, 0.5f, 0.1f, 1.0f), "(unpublished)");
                        if (ImGui::IsItemHovered()) {
                            ImGui::BeginTooltip();
                            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                            ImGui::TextWrapped("This pack needs to be loaded in-game to sync with a published code");
                            ImGui::PopTextWrapPos();
                            ImGui::EndTooltip();
                        }
                    }
                    else {
                        ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.3f, 1.0f), "(published)");
                    }

                    
                    if (isOpen) {
                        ImGui::Indent(10.0f);
                        ImGui::Spacing();

                        
                        ImGui::Columns(2, "PackInfoColumns", false);
                        ImGui::SetColumnWidth(0, 200);

                       
                        ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "Code:");
                        ImGui::NextColumn();
                        if (!packData.code.empty()) {
                            ImGui::TextUnformatted(packData.code.c_str());
                            ImGui::SameLine(0, 5.0f); 

                            bool justCopiedPackCode = (lastCopiedPackCode == packData.code) && (packCodeCopyFlashTimer > 0.0f);
                            std::string copyButtonId = "CopyCode##" + packKey;
                            std::string copyButtonText = justCopiedPackCode ? "Copied!" : "Copy";

                            if (justCopiedPackCode) {
                                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.7f, 0.0f, 0.8f)); 
                                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.7f, 0.0f, 0.9f)); 
                            }

                            if (ImGui::Button(copyButtonText.c_str(), ImVec2(justCopiedPackCode ? 70 : 60, 0))) { 
                                ImGui::SetClipboardText(packData.code.c_str());
                                lastCopiedPackCode = packData.code;
                                packCodeCopyFlashTimer = 2.0f;
                            }

                            if (justCopiedPackCode) {
                                ImGui::PopStyleColor(2);
                            }
                        }
                        else {
                            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Not Available");
                        }
                        ImGui::NextColumn();

                        
                        ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "Shots:");
                        ImGui::NextColumn();
                        ImGui::Text("%d", packData.numShots);
                        ImGui::NextColumn();

                        ImGui::Columns(1);
                        ImGui::Spacing();

                        
                        ImGui::Separator();
                        ImGui::Spacing();

                        float buttonWidth = 120.0f;
                        float availWidth = ImGui::GetContentRegionAvail().x;
                        float totalButtonWidth = buttonWidth * 2 + ImGui::GetStyle().ItemSpacing.x;
                        ImGui::SetCursorPosX((availWidth - totalButtonWidth) * 0.5f + ImGui::GetCursorPosX());

                        
                        if (!packData.code.empty()) {
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 0.3f, 1.0f));
                            if (ImGui::Button("Play Pack", ImVec2(buttonWidth, 30))) {
                                std::string cmd = "load_training " + packData.code;
                                gameWrapper->Execute([this, cmd](GameWrapper* gw) {
                                    cvarManager->executeCommand(cmd, false);
                                    });
                            }
                            ImGui::PopStyleColor(2);
                        }
                        else {
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 0.7f));
                            ImGui::Button("Play Pack", ImVec2(buttonWidth, 30));
                            ImGui::PopStyleColor();
                            if (ImGui::IsItemHovered()) {
                                ImGui::SetTooltip("Code required to play");
                            }
                        }

                        ImGui::SameLine();

                        std::string deletePopupId = "DeleteConfirmPopup##" + packKey; 
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.15f, 0.15f, 0.9f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.25f, 0.25f, 0.9f));
                        if (ImGui::Button("Delete Pack", ImVec2(buttonWidth, 30))) {
                            deletePackKey = packKey; 
                            ImGui::OpenPopup(deletePopupId.c_str());
                        }
                        ImGui::PopStyleColor(2);

                        
                        if (ImGui::BeginPopupModal(deletePopupId.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                            ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
                            ImGui::TextColored(ImVec4(0.9f, 0.2f, 0.2f, 1.0f), "Confirm Pack Deletion");
                            ImGui::PopFont();
                            ImGui::Separator();

                            ImGui::TextWrapped("Are you sure you want to delete this pack?");
                            
                            ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "Pack: %s", packData.name.c_str());
                            
                            ImGui::Spacing();
                            ImGui::Separator();
                            ImGui::Spacing();

                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.15f, 0.15f, 0.9f));
                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.25f, 0.25f, 0.9f));
                            if (ImGui::Button("Delete Pack##ConfirmDeleteInLoop", ImVec2(120, 30))) { // Unique ID for button
                                if (!deletePackKey.empty() && trainingData->count(deletePackKey) > 0) {
                                    
                                    LOG("Deleting training pack: {}", trainingData->at(deletePackKey).name);

                                    std::filesystem::path packFolderBase = myDataFolder / "TrainingPacks";
                                    
                                    std::string packIdentifier = trainingData->at(deletePackKey).code.empty() ?
                                        trainingData->at(deletePackKey).name : trainingData->at(deletePackKey).code;
                                   
                                    if (trainingData->at(deletePackKey).code.empty()) {
                                        packIdentifier = storageManager.recordingStorage.sanitizeFilename(packIdentifier);
                                    }

                                    std::filesystem::path specificPackFolder = packFolderBase / packIdentifier;

                                    if (std::filesystem::exists(specificPackFolder)) {
                                        try {
                                            std::filesystem::remove_all(specificPackFolder);
                                            LOG("Successfully deleted folder: {}", specificPackFolder.string());
                                        }
                                        catch (const std::filesystem::filesystem_error& e) {
                                            LOG("Error deleting folder {}: {}", specificPackFolder.string(), e.what());
                                        }
                                    } else {
                                        LOG("Pack folder not found for deletion: {}", specificPackFolder.string());
                                    }

                                    trainingData->erase(deletePackKey);

                                    
                                    for (auto& [key_iter, value_iter] : *trainingData) { 
                                        shiftToPositive(value_iter);
                                    }
                                    storageManager.saveCompressedTrainingDataWithRecordings(*trainingData, myDataFolder);
                             
                                }

                                deletePackKey = "";
                                ImGui::CloseCurrentPopup();
                            }
                            ImGui::PopStyleColor(2);

                            ImGui::SameLine();
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 0.8f));
                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.4f, 0.4f, 0.8f));
                            if (ImGui::Button("Cancel##ConfirmDeleteInLoop", ImVec2(120, 30))) { 
                                deletePackKey = ""; 
                                ImGui::CloseCurrentPopup();
                            }
                            ImGui::PopStyleColor(2);

                            ImGui::EndPopup();
                        }


                        ImGui::Spacing();
                        ImGui::Unindent(10.0f);
                    }

                    ImGui::PopID();
                    ImGui::Separator();
                }
            }

            ImGui::EndChild();
            ImGui::PopStyleColor();
            ImGui::PopStyleVar();

            ImGui::EndTabItem();
        }

        
        if (ImGui::BeginTabItem("Current Training Pack")) {
            ImGui::Spacing();

            
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f, 0.2f, 0.3f, 0.3f));
            ImGui::BeginChild("CurrentPackHeader", ImVec2(ImGui::GetContentRegionAvail().x, 60), true);

            ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
            ImGui::TextColored(ImVec4(0.9f, 0.9f, 1.0f, 1.0f), "Current Pack:");
            ImGui::SameLine();
            if (currentTrainingData.name.empty()) {
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "No pack loaded");
            }
            else {
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.8f, 1.0f), "%s", currentTrainingData.name.c_str());
                ImGui::SameLine(ImGui::GetWindowWidth() - 150);
                ImGui::TextColored(ImVec4(0.8f, 0.8f, 1.0f, 1.0f), "Shots: %d", static_cast<int>(currentTrainingData.shots.size()));
            }
            ImGui::PopFont();

           
            if (!currentTrainingData.code.empty()) {
                ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "Code: %s", currentTrainingData.code.c_str());
            }

            ImGui::EndChild();
            ImGui::PopStyleColor();

            ImGui::Spacing();

            
            if (currentTrainingData.shots.size() > 0) {
                ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "Shot Details:");
                ImGui::Separator();
                ImGui::Spacing();

                ImVec2 shotsAreaSize = ImGui::GetContentRegionAvail();
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.15f, 0.15f, 0.15f, 0.5f));
                ImGui::BeginChild("ShotsScrollArea", shotsAreaSize, true);

                for (size_t i = 0; i < currentTrainingData.shots.size(); ++i) {
                    ImGui::PushID(static_cast<int>(i));

                    
                    bool isCurrentEditedShot = (i == currentTrainingData.currentEditedShot);
                    ImGuiTreeNodeFlags flags = isCurrentEditedShot ? ImGuiTreeNodeFlags_DefaultOpen : 0;

                    std::string shotName = "Shot " + std::to_string(i + 1);
                    if (isCurrentEditedShot) {
                        shotName += " (Current)";
                        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.3f, 0.3f, 0.1f, 0.9f));
                        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.4f, 0.4f, 0.2f, 0.9f));
                    }
                    else {
                        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.2f, 0.2f, 0.9f));
                        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.3f, 0.3f, 0.3f, 0.9f));
                    }

                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 8));
                    bool isOpen = ImGui::CollapsingHeader(shotName.c_str(), flags);
                    ImGui::PopStyleVar();
                    ImGui::PopStyleColor(2);

                    if (isOpen) {
                        ImGui::Indent(10.0f);
                        ShotState& shot = currentTrainingData.shots[i];

                        ImGui::Columns(2, "ShotDetailsColumns", false);
                        ImGui::SetColumnWidth(0, 180);

                        
                        ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "Car Velocity:");
                        ImGui::NextColumn();
                        ImGui::Text("(%.1f, %.1f, %.1f)", shot.extendedStartingVelocity.X,
                            shot.extendedStartingVelocity.Y,
                            shot.extendedStartingVelocity.Z);
                        ImGui::NextColumn();

                        
                        ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "Boost Amount:");
                        ImGui::NextColumn();
                        ImGui::Text("%d", shot.boostAmount);
                        ImGui::NextColumn();

                        
                        ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "Starting Velocity:");
                        ImGui::NextColumn();
                        ImGui::Text("%d", shot.startingVelocity);
                        ImGui::NextColumn();

                        
                        ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "Car Freeze State:");
                        ImGui::NextColumn();
                        ImGui::TextColored(!shot.freezeCar ? ImVec4(1.0f, 0.5f, 0.0f, 1.0f) : ImVec4(0.0f, 0.8f, 0.2f, 1.0f),
                            shot.freezeCar ? "Frozen" : "Unfrozen");
                        ImGui::NextColumn();

                        ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "Jump State:");
                        ImGui::NextColumn();
                        ImGui::TextColored(!shot.hasJump ? ImVec4(1.0f, 0.5f, 0.0f, 1.0f) : ImVec4(0.0f, 0.8f, 0.2f, 1.0f),
                            !shot.hasJump ? "No Jump" : "Has Jump");
                        ImGui::NextColumn();

                        ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "Has Recording ?");
                        ImGui::NextColumn();
                        ImGui::TextColored(!shot.recording.inputs.empty() ? ImVec4(0.0f, 0.8f, 0.2f, 1.0f) : ImVec4(1.0f, 0.5f, 0.0f, 1.0f),
                            !shot.recording.inputs.empty() ? "Yes" : "No");
                        ImGui::NextColumn();

                        ImGui::Columns(1);

                        
                        if (shot.goalAnchors.first || shot.goalAnchors.second) {
                            ImGui::Spacing();
                            ImGui::Separator();

                            ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.2f, 1.0f), "Goal Blocker Settings:");
                            ImGui::Spacing();

                            ImGui::Columns(2, "GoalBlockerColumns", false);
                            ImGui::SetColumnWidth(0, 180);

                            
                            ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "Anchor Point 1:");
                            ImGui::NextColumn();
                            ImGui::Text("(%.1f, %.1f, %.1f)", shot.goalBlocker.first.X,
                                shot.goalBlocker.first.Y,
                                shot.goalBlocker.first.Z);
                            ImGui::NextColumn();

                           
                            ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "Anchor Point 2:");
                            ImGui::NextColumn();
                            ImGui::Text("(%.1f, %.1f, %.1f)", shot.goalBlocker.second.X,
                                shot.goalBlocker.second.Y,
                                shot.goalBlocker.second.Z);
                            ImGui::NextColumn();

                            ImGui::Columns(1);
                        }

                        ImGui::Unindent(10.0f);
                    }

                    ImGui::PopID();
                    ImGui::Separator();
                }

                ImGui::EndChild();
                ImGui::PopStyleColor();
                ImGui::PopStyleVar();
            }
            else {
                ImGui::Spacing();
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
                ImGui::TextWrapped("No shots available in this pack. Load a training pack to view its details.");
                ImGui::PopStyleColor();
            }

            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Snapshot Gallery")) {
            ImGui::Spacing();
            if (ImGui::BeginTabBar("SnapshotGallerySubTabs")) {
                if (ImGui::BeginTabItem("All Snapshots")) {
                    ImGui::Spacing();
                    RenderAllSnapshotsTab(); 
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Grouped Snapshots")) {
                    ImGui::Spacing();
                    RenderGroupedSnapshotsTab(); 
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::EndTabItem();
        }
        
        ImGui::EndTabBar();
    }
       
   
    style.ItemSpacing.y = origItemSpacing;
    style.FramePadding.y = origFramePadding;
}