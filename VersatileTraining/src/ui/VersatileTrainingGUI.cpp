#include "pch.h"
#include "src/core/VersatileTraining.h"

void VersatileTraining::RenderSettings() {
	ImGui::Text("Versatile Training Settings");
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
        // Custom Training Pack tab
        if (ImGui::BeginTabItem("Custom Training Pack")) {
            // Content for Custom Training Pack tab
            ImGui::Text("Custom Training Pack Settings");

            // You can add your custom training pack controls here
            static char trainingCode[20];
            ImGui::InputText("Training Code", trainingCode, IM_ARRAYSIZE(trainingCode));

            // Add more controls as needed...

            ImGui::EndTabItem();
        }

        // Snapshot Gallery tab
        if (ImGui::BeginTabItem("Snapshot Gallery")) {
            ImGui::Text("Saved Snapshots (%d)", snapshotManager.replayStates.size());
            ImGui::Separator();

            ImVec2 availableSize = ImGui::GetContentRegionAvail();
            ImVec2 childSize = ImVec2(availableSize.x, availableSize.y);
            ImGui::BeginChild("SnapshotScrollArea", childSize, true, ImGuiWindowFlags_HorizontalScrollbar);

            for (size_t i = 0; i < snapshotManager.replayStates.size(); ++i) {
                ReplayState& state = snapshotManager.replayStates[i];
                ImGui::PushID(static_cast<int>(i));

                const std::string& name = state.replayName.empty() ? "Unnamed Snapshot" : state.replayName;
                if (ImGui::CollapsingHeader(name.c_str())) {
                    ImGui::Text("Saved At: %s", state.formattedTimeStampOfSaved.c_str());

                    if (state.captureSource == CaptureSource::Replay) {
                        ImGui::Text("Replay Time: %s", state.replayTime.c_str());
                        ImGui::Text("Time Remaining: %s", state.timeRemainingInGame.c_str());
                        ImGui::Text("Player: %s", state.focusPlayerName.c_str());
                        
                    }
                    ImGui::Text("Boost: %d", state.boostAmount);
                    // Always show car data
                    if (ImGui::TreeNode("Car Details")) {
                        ImGui::Text("Location: (%.1f, %.1f, %.1f)", state.carLocation.X, state.carLocation.Y, state.carLocation.Z);
                        ImGui::Text("Velocity: (%.1f, %.1f, %.1f)", state.carVelocity.X, state.carVelocity.Y, state.carVelocity.Z);
                        ImGui::Text("Angular Velocity: (%.1f, %.1f, %.1f)", state.carAngularVelocity.X, state.carAngularVelocity.Y, state.carAngularVelocity.Z);
                        ImGui::Text("Rotation: (Pitch: %d, Yaw: %d, Roll: %d)", state.carRotation.Pitch, state.carRotation.Yaw, state.carRotation.Roll);
                        ImGui::TreePop();
                    }

                    // Show ball data if training or replay has it
                    if (ImGui::TreeNode("Ball Details")) {
                        ImGui::Text("Location: (%.1f, %.1f, %.1f)", state.ballLocation.X, state.ballLocation.Y, state.ballLocation.Z);
                        ImGui::Text("Shot Speed: %.1f", state.ballSpeed);
                        ImGui::Text("Shot Direction: (Pitch: %d, Yaw: %d)", state.ballRotation.Pitch, state.ballRotation.Yaw);
                        ImGui::TreePop();
                    }

                    // Action buttons
                    if (ImGui::Button("Load")) {
                        savedReplayState = state;
                        savedReplayState.ballSet = false;
                        savedReplayState.carLocationSet = false;
                        savedReplayState.carRotationSet = false;
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Delete")) {
                        snapshotManager.replayStates.erase(snapshotManager.replayStates.begin() + i);
                        --i;
                        ImGui::PopID();
                        continue;
                    }
                }

                ImGui::PopID();
                ImGui::Separator();
            }

            ImGui::EndChild();
            ImGui::EndTabItem();
        }


        ImGui::EndTabBar();
    }

}