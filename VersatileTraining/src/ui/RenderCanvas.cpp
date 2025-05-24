#include "pch.h"
#include "src/core/VersatileTraining.h"
void VersatileTraining::Render(CanvasWrapper canvas) {




    if (settingsOpen) return;

    

    RenderVelocityOfCar(canvas);

    if (!(isInTrainingPack() || isInTrainingEditor())) return;

    std::vector<std::string> statusLines;
    bool shouldDrawStatusPanel = false;

    auto getBoundKeyOrUnbound = [&](const std::string& command) -> std::string {
        auto it = currentBindings.find(command);
        if (it != currentBindings.end() && !it->second.empty()) {
            return it->second;
        }
        return "Unbound";
    };

    if (!shotReplicationManager.testCalledInStartRound && isInTrainingPack() && currentTrainingData.customPack) {
        shouldDrawStatusPanel = true;
        if (recordingEnabled) {
            statusLines.push_back(currentShotState.recording.inputs.empty() ? "No Recording Available" : "Has Recording");
        }
        statusLines.push_back(currentShotState.hasJump ? "Jump: Enabled" : "Jump: Disabled");
    }

    if (editingVariances) {
        shouldDrawStatusPanel = true;
        statusLines.push_back("Boost: " + std::to_string(currentShotState.boostAmount));

        std::string rotKey = getBoundKeyOrUnbound("unlockCar");
        statusLines.push_back(lockRotation ? "Rotation: Locked (" + rotKey + ")" : "Rotation: Unlocked (" + rotKey + ")");

        std::string freezeKey = getBoundKeyOrUnbound("freezeCar");
        statusLines.push_back(currentShotState.freezeCar ? "Car: Frozen (" + freezeKey + ")" : "Car: Unfrozen (" + freezeKey + ")");

        std::string jumpKey = getBoundKeyOrUnbound("removeJump");
        statusLines.push_back(currentShotState.hasJump ? "Jump: Enabled (" + jumpKey + ")" : "Jump: Disabled (" + jumpKey + ")");

        std::string velDirKey = getBoundKeyOrUnbound("lockStartingVelocity");
        statusLines.push_back(!unlockStartingVelocity ? "Velocity Dir: Locked (" + velDirKey + ")" : "Velocity Dir: Unlocked (" + velDirKey + ")");
        
        statusLines.push_back("Velocity Val: " + std::to_string(currentShotState.startingVelocity));
        
        if (isInTrainingEditor()) {
            if (isCarRotatable) {
                if (!playTestStarted) {
                    std::string sceneKey = getBoundKeyOrUnbound("lockScene");
                    statusLines.push_back(lockScene ? "Scene: Locked (" + sceneKey + ")" : "Scene: Unlocked (" + sceneKey + ")");
                    
                }
                else {
                    statusLines.push_back(lockScene ? "Scene: Locked" : "Scene: Unlocked");
                    
                }
            }
        }
    } else if (!editingGoalBlocker && isInTrainingEditor()) { 
        if (!playTestStarted) {
            shouldDrawStatusPanel = true;
            std::string sceneKey = getBoundKeyOrUnbound("lockScene");
            statusLines.push_back(lockScene ? "Scene: Locked (" + sceneKey + ")" : "Scene: Unlocked (" + sceneKey + ")");
            
        } else {
            shouldDrawStatusPanel = true;
            if (!shotReplicationManager.testCalledInStartRound) {
                statusLines.push_back(currentShotState.hasJump ? "Jump: Enabled" : "Jump: Disabled");
            }
            if (recordingEnabled) {
                statusLines.push_back(shotReplicationManager.recording ? "Recording: On" : "Recording: Off");
            }
        }
    }

    if (shouldDrawStatusPanel && !statusLines.empty()) {
        Vector2 screenSize = canvas.GetSize();
        float panelPadding = 10.0f;
        float lineHeight = 18.0f;
        float textScale = 1.2f;
        
        float panelWidth = 320.0f; 


        float panelHeight = (statusLines.size() * lineHeight) + (panelPadding * 2);
        
        Vector2F panelPos = {screenSize.X - panelWidth - panelPadding, screenSize.Y - panelHeight - panelPadding};

        LinearColor panelBgColor = {0, 0, 0, 150};
        canvas.SetColor(panelBgColor);
        canvas.SetPosition(panelPos);
        canvas.FillBox(Vector2F{panelWidth, panelHeight});

        LinearColor textColor = {255, 255, 255, 220};
        canvas.SetColor(textColor);
        Vector2F textPos = {panelPos.X + panelPadding, panelPos.Y + panelPadding};

        for (const auto& line : statusLines) {
            canvas.SetPosition(textPos);
            canvas.DrawString(line, textScale, textScale, false);
            textPos.Y += lineHeight;
        }
    }
    if (editingGoalBlocker) {
        CameraWrapper cam = gameWrapper->GetCamera();
        if (cam.IsNull()) return;

        Vector camLoc = cam.GetLocation();
        Vector forward = GetForwardVectorFromRotator(cam.GetRotation());
        RT::Frustum frust{ canvas, cam };

        Vector projectedPoint;
        if (ProjectToPlaneY(camLoc, forward, (float)backWall, projectedPoint)) {
            
            Vector2 screenPoint = canvas.Project(projectedPoint);
            int crossSize = 5;
            canvas.SetColor((char)255, (char)255, (char)255, (char)255);
            canvas.DrawLine(screenPoint - Vector2(crossSize, 0), screenPoint + Vector2(crossSize, 0), 1.0f);
            canvas.DrawLine(screenPoint - Vector2(0, crossSize), screenPoint + Vector2(0, crossSize), 1.0f);

            
            if (saveCursorPos) {
                 
                 
                 
                if (!currentShotState.goalAnchors.first) {
                     
                    currentShotState.goalBlocker.first = projectedPoint;
                    saveCursorPos = false;
                    rectangleMade = false;
                    rectangleSaved = false;
                    currentShotState.goalAnchors.first = true;
                     
                }
                else if (!currentShotState.goalAnchors.second) {
                    float distanceX = abs(projectedPoint.X - currentShotState.goalBlocker.first.X);
                    float distanceZ = abs(projectedPoint.Z - currentShotState.goalBlocker.first.Z);
                    
                    
                    float minSize = 75.0f; 
                    
                    if (distanceX < minSize || distanceZ < minSize) {
                        
                        LOG("Rectangle too small (X width: {}, Z height: {}). Minimum size is {}. Clearing goal blocker.", 
                            distanceX, distanceZ, minSize);
                        currentShotState.goalBlocker.first = Vector(0, 0, 0);
                        currentShotState.goalBlocker.second = Vector(0, 0, 0);
                        currentShotState.goalAnchors.first = false;
                        currentShotState.goalAnchors.second = false;
                        rectangleMade = false;
                        saveCursorPos = false;
                        rectangleSaved = false;
                    } else {
                        currentShotState.goalBlocker.second = projectedPoint;
                        rectangleSaved = true;
                        saveCursorPos = false;
                        currentShotState.goalAnchors.second = true;
                        LOG("Created goal blocker: first anchor point: X {}, Z {}, second anchor point: X {} Z {}", 
                            currentShotState.goalBlocker.first.X, currentShotState.goalBlocker.first.Z, 
                            currentShotState.goalBlocker.second.X, currentShotState.goalBlocker.second.Z);
                    }
                }
                else {
                     
                    currentShotState.goalBlocker.first = projectedPoint;
                    currentShotState.goalBlocker.second = { 0, 0, 0 };
                    currentShotState.goalAnchors.first = true;
                    currentShotState.goalAnchors.second = false;
                    rectangleMade = false;
                    saveCursorPos = false;
                    rectangleSaved = false;
                }
            }
            else if (currentShotState.goalAnchors.first && !currentShotState.goalAnchors.second) {
                currentShotState.goalBlocker.second = projectedPoint;
                rectangleMade = true;
            }

            if (currentShotState.goalAnchors.first && currentShotState.goalAnchors.second) {
                rectangleMade = true;
            }

           
            if (rectangleMade) {
                Vector topLeft(max(currentShotState.goalBlocker.first.X, currentShotState.goalBlocker.second.X), backWall, max(currentShotState.goalBlocker.first.Z, currentShotState.goalBlocker.second.Z));
                Vector topRight(min(currentShotState.goalBlocker.first.X, currentShotState.goalBlocker.second.X), backWall, max(currentShotState.goalBlocker.first.Z, currentShotState.goalBlocker.second.Z));
                Vector bottomLeft(max(currentShotState.goalBlocker.first.X, currentShotState.goalBlocker.second.X), backWall, min(currentShotState.goalBlocker.first.Z, currentShotState.goalBlocker.second.Z));
                Vector bottomRight(min(currentShotState.goalBlocker.first.X, currentShotState.goalBlocker.second.X), backWall, min(currentShotState.goalBlocker.first.Z, currentShotState.goalBlocker.second.Z));

                // Get ball data for the preview editor
                ServerWrapper gameState = gameWrapper->GetCurrentGameState();
                BallWrapper ball = gameState.GetBall();
               

                if (currentShotState.goalAnchors.first && currentShotState.goalAnchors.second) {
                    RenderInvertedGoalBlocker_OutlinedAndGridded(canvas, cam, frust, ball, topLeft, topRight, bottomLeft, bottomRight);
                }
                else {
                    canvas.SetColor((char)0, (char)255, (char)0, (char)200);
                    RT::Line lineTop(topLeft, topRight, RT::GetVisualDistance(canvas, frust, cam, topLeft) * 1.5f);
                    RT::Line lineRight(topRight, bottomRight, RT::GetVisualDistance(canvas, frust, cam, topRight) * 1.5f);
                    RT::Line lineBottom(bottomRight, bottomLeft, RT::GetVisualDistance(canvas, frust, cam, bottomRight) * 1.5f);
                    RT::Line lineLeft(bottomLeft, topLeft, RT::GetVisualDistance(canvas, frust, cam, bottomLeft) * 1.5f);

                    lineTop.thickness = goalBlockerOutlineThickness;
                    lineRight.thickness = goalBlockerOutlineThickness;
                    lineBottom.thickness = goalBlockerOutlineThickness;
                    lineLeft.thickness = goalBlockerOutlineThickness;

                    lineTop.DrawWithinFrustum(canvas, frust);
                    lineRight.DrawWithinFrustum(canvas, frust);
                    lineBottom.DrawWithinFrustum(canvas, frust);
                    lineLeft.DrawWithinFrustum(canvas, frust);
                }
                 
                // if (!ball.IsNull()) {
                //    Vector2 ballScreenPos = canvas.Project(ball.GetLocation());
                //    float ballRadius = Distance(ballScreenPos, canvas.Project(ball.GetLocation() + Vector(92.75f, 0, 0)));

                //    // Draw enhanced goal blocker with grid and gradients
                //    RenderEnhancedGoalBlocker(canvas, cam, frust, ball, topLeft, topRight, bottomLeft, bottomRight);
                //}
                //else {
                //    
                //    canvas.SetColor((char)0, (char)255, (char)0, (char)200);
                //    RT::Line lineTop(topLeft, topRight, RT::GetVisualDistance(canvas, frust, cam, topLeft) * 1.5f);
                //    RT::Line lineRight(topRight, bottomRight, RT::GetVisualDistance(canvas, frust, cam, topRight) * 1.5f);
                //    RT::Line lineBottom(bottomRight, bottomLeft, RT::GetVisualDistance(canvas, frust, cam, bottomRight) * 1.5f);
                //    RT::Line lineLeft(bottomLeft, topLeft, RT::GetVisualDistance(canvas, frust, cam, bottomLeft) * 1.5f);

                //    lineTop.DrawWithinFrustum(canvas, frust);
                //    lineRight.DrawWithinFrustum(canvas, frust);
                //    lineBottom.DrawWithinFrustum(canvas, frust);
                //    lineLeft.DrawWithinFrustum(canvas, frust);
                //}
            }
        }

        // Optional: Draw center screen cursor
        Vector2 screenSize = canvas.GetSize();
        Vector2 center(screenSize.X / 2, screenSize.Y / 2);
        canvas.SetColor((char)255, (char)255, (char)255, (char)255);
        int length = 10;
        canvas.DrawLine(Vector2(center.X - length, center.Y), Vector2(center.X + length, center.Y), 1.5f);
        canvas.DrawLine(Vector2(center.X, center.Y - length), Vector2(center.X, center.Y + length), 1.5f);
    }else{

    // If we don't have a complete goal blocker, return early
    if (!(currentShotState.goalAnchors.first && currentShotState.goalAnchors.second)) return;

    CameraWrapper camera = gameWrapper->GetCamera();
    if (camera.IsNull()) return;

    RT::Frustum frustum{ canvas, camera };
    ServerWrapper gameState = gameWrapper->GetCurrentGameState();
    BallWrapper ball = gameState.GetBall();
    if (ball.IsNull()) return;

    // Render the enhanced goal blocker during normal gameplay
    Vector topLeft(max(currentShotState.goalBlocker.first.X, currentShotState.goalBlocker.second.X), backWall, max(currentShotState.goalBlocker.first.Z, currentShotState.goalBlocker.second.Z));
    Vector topRight(min(currentShotState.goalBlocker.first.X, currentShotState.goalBlocker.second.X), backWall, max(currentShotState.goalBlocker.first.Z, currentShotState.goalBlocker.second.Z));
    Vector bottomLeft(max(currentShotState.goalBlocker.first.X, currentShotState.goalBlocker.second.X), backWall, min(currentShotState.goalBlocker.first.Z, currentShotState.goalBlocker.second.Z));
    Vector bottomRight(min(currentShotState.goalBlocker.first.X, currentShotState.goalBlocker.second.X), backWall, min(currentShotState.goalBlocker.first.Z, currentShotState.goalBlocker.second.Z));


        // RenderEnhancedGoalBlocker(canvas, camera, frustum, ball, topLeft, topRight, bottomLeft, bottomRight);

        RenderInvertedGoalBlocker_OutlinedAndGridded(canvas, camera, frustum, ball, topLeft, topRight, bottomLeft, bottomRight);
    }
}