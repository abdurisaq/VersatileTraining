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

		//InitDirectInput(hwnd);
	}

	static char trainingCode[20];
	ImGui::InputText("custom Training code",trainingCode, IM_ARRAYSIZE(trainingCode));

	
	ImGui::SliderInt("Boost Amount", &currentShotState.boostAmount, currentTrainingData.boostMin, currentTrainingData.boostMax);
	ImGui::SliderInt("Starting Velocity", &currentShotState.startingVelocity, currentTrainingData.minVelocity, currentTrainingData.maxVelocity);
	//ImGui::DragIntRange2("Starting Velocity", &startingVelocityMin, &startingVelocityMax, 1, 0, 100, "%d", "%d");
	/*ImGui::RangeSliderInt("Starting Velocity", &startingVelocityMin, &startingVelocityMax, minVelocity, maxVelocity, "(%d,%d)");*/

	/*if (ImGui::Button("Submit")) {
		CustomTrainingData data;
		data.code = trainingCode;
		data.boostAmounts.push_back(boostAmount);
		data.startingVelocity.push_back(startingVelocity);
		trainingData.insert_or_assign(trainingCode,data);
	}*/

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

//void VersatileTraining::RenderWindow() {
//
//
//}