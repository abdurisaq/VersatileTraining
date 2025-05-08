#include "pch.h"
#include "src/core/VersatileTraining.h"
void VersatileTraining::Render(CanvasWrapper canvas) {
    if (settingsOpen) return;
    RenderVelocityOfCar(canvas);

    if (!(isInTrainingPack() || isInTrainingEditor())) return;
    LinearColor colors;
    colors.R = 255;
    colors.G = 255;
    colors.B = 0;
    colors.A = 255;
    canvas.SetColor(colors);
    canvas.SetPosition(Vector2F{ 1400.0, 1040.0 });
    if (lockScene && isInTrainingEditor()&& !playTestStarted) {
        canvas.DrawString("Scene locked, press V to unlock", 2.0, 2.0, false);
    }
    else if (isInTrainingEditor()&& !playTestStarted) {
        canvas.DrawString("Scene unlocked, press V to lock", 2.0, 2.0, false);
    }

    if (editingVariances) {
        canvas.SetPosition(Vector2F{ 0.0, 0.0 });
        canvas.DrawString("Boost Amount: " + std::to_string(currentShotState.boostAmount), 2.0, 2.0, false);
        canvas.SetPosition(Vector2F{ 0.0, 20.0 });
        if (lockRotation) {
            canvas.DrawString("Car rotation locked, press X to unlock", 2.0, 2.0, false);
        }
        else {
            canvas.DrawString("Car rotation unlocked, press X to lock", 2.0, 2.0, false);
        }
        canvas.SetPosition(Vector2F{ 0.0, 40.0 });
        if (currentShotState.freezeCar) {
            canvas.DrawString("Car frozen, press F to unfreeze", 2.0, 2.0, false);
        }
        else {
            canvas.DrawString("Car unfrozen, press F to freeze", 2.0, 2.0, false);
        }
        canvas.SetPosition(Vector2F{ 0.0, 60.0 });
        if (currentShotState.hasJump) {
            canvas.DrawString("Jump available, press J to disable", 2.0, 2.0, false);
        }
        else {
            canvas.DrawString("Jump disabled, press J to enable", 2.0, 2.0, false);
        }
        canvas.SetPosition(Vector2F{ 0.0, 80.0 });
        if (!unlockStartingVelocity) {
            canvas.DrawString("Starting velocity locked, press B to unlock", 2.0, 2.0, false);
        }
        else {
            canvas.DrawString("Starting velocity unlocked, press B to lock", 2.0, 2.0, false);
        }
        canvas.SetPosition(Vector2F{ 0.0, 100.0 });
        canvas.DrawString("Starting Velocity: " + std::to_string(currentShotState.startingVelocity), 2.0, 2.0, false);
    }
    else if (editingGoalBlocker) {
        CameraWrapper cam = gameWrapper->GetCamera();
        if (cam.IsNull()) return;

        Vector camLoc = cam.GetLocation();
        Vector forward = GetForwardVectorFromRotator(cam.GetRotation());
        RT::Frustum frust{ canvas, cam };

        Vector projectedPoint;
        if (ProjectToPlaneY(camLoc, forward, (float)backWall, projectedPoint)) {
            // Draw projected point as a small cross
            Vector2 screenPoint = canvas.Project(projectedPoint);
            int crossSize = 5;
            canvas.SetColor((char)255, (char)255, (char)255, (char)255);
            canvas.DrawLine(screenPoint - Vector2(crossSize, 0), screenPoint + Vector2(crossSize, 0), 1.0f);
            canvas.DrawLine(screenPoint - Vector2(0, crossSize), screenPoint + Vector2(0, crossSize), 1.0f);

            // Handle mouse input
            if (saveCursorPos) {
                LOG("current anchors first: {}, second: {}", currentShotState.goalAnchors.first ? "true" : "false", currentShotState.goalAnchors.second ? "true" : "false");
                LOG("current anchor firs : X {} Z {}, second : X {} Z {}", currentShotState.goalBlocker.first.X, currentShotState.goalBlocker.first.Z, currentShotState.goalBlocker.second.X, currentShotState.goalBlocker.second.Z);
                LOG("projected point : X {} Z {}", projectedPoint.X, projectedPoint.Z);
                if (!currentShotState.goalAnchors.first) {
                    LOG("entered this function");
                    currentShotState.goalBlocker.first = projectedPoint;
                    saveCursorPos = false;
                    rectangleMade = false;
                    rectangleSaved = false;
                    currentShotState.goalAnchors.first = true;
                    LOG("adding to first anchor point : X {}, Z {}", currentShotState.goalBlocker.first.X, currentShotState.goalBlocker.first.Z);
                }
                else if (!currentShotState.goalAnchors.second) {
                    currentShotState.goalBlocker.second = projectedPoint;
                    rectangleSaved = true;
                    saveCursorPos = false;
                    currentShotState.goalAnchors.second = true;

                    LOG("first anchor point : X {}, Z {}, second anchor point : X {} Z {}", currentShotState.goalBlocker.first.X, currentShotState.goalBlocker.first.Z, currentShotState.goalBlocker.second.X, currentShotState.goalBlocker.second.Z);
                }
                else {
                    LOG("this else call is being called");
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

            // If we have both points, draw rectangle on the goal plane
            if (rectangleMade) {
                Vector topLeft(max(currentShotState.goalBlocker.first.X, currentShotState.goalBlocker.second.X), backWall, max(currentShotState.goalBlocker.first.Z, currentShotState.goalBlocker.second.Z));
                Vector topRight(min(currentShotState.goalBlocker.first.X, currentShotState.goalBlocker.second.X), backWall, max(currentShotState.goalBlocker.first.Z, currentShotState.goalBlocker.second.Z));
                Vector bottomLeft(max(currentShotState.goalBlocker.first.X, currentShotState.goalBlocker.second.X), backWall, min(currentShotState.goalBlocker.first.Z, currentShotState.goalBlocker.second.Z));
                Vector bottomRight(min(currentShotState.goalBlocker.first.X, currentShotState.goalBlocker.second.X), backWall, min(currentShotState.goalBlocker.first.Z, currentShotState.goalBlocker.second.Z));

                // Get ball data for the preview editor
                ServerWrapper gameState = gameWrapper->GetCurrentGameState();
                BallWrapper ball = gameState.GetBall();
                if (!ball.IsNull()) {
                    Vector2 ballScreenPos = canvas.Project(ball.GetLocation());
                    float ballRadius = Distance(ballScreenPos, canvas.Project(ball.GetLocation() + Vector(92.75f, 0, 0)));

                    // Draw enhanced goal blocker with grid and gradients
                    RenderEnhancedGoalBlocker(canvas, cam, frust, ball, topLeft, topRight, bottomLeft, bottomRight);
                }
                else {
                    // Fallback if ball is not available - simple rectangle
                    canvas.SetColor((char)0, (char)255, (char)0, (char)200);
                    RT::Line lineTop(topLeft, topRight, RT::GetVisualDistance(canvas, frust, cam, topLeft) * 1.5f);
                    RT::Line lineRight(topRight, bottomRight, RT::GetVisualDistance(canvas, frust, cam, topRight) * 1.5f);
                    RT::Line lineBottom(bottomRight, bottomLeft, RT::GetVisualDistance(canvas, frust, cam, bottomRight) * 1.5f);
                    RT::Line lineLeft(bottomLeft, topLeft, RT::GetVisualDistance(canvas, frust, cam, bottomLeft) * 1.5f);

                    lineTop.DrawWithinFrustum(canvas, frust);
                    lineRight.DrawWithinFrustum(canvas, frust);
                    lineBottom.DrawWithinFrustum(canvas, frust);
                    lineLeft.DrawWithinFrustum(canvas, frust);
                }
            }
        }

        // Optional: Draw center screen cursor
        Vector2 screenSize = canvas.GetSize();
        Vector2 center(screenSize.X / 2, screenSize.Y / 2);
        canvas.SetColor((char)255, (char)255, (char)255, (char)255);
        int length = 10;
        canvas.DrawLine(Vector2(center.X - length, center.Y), Vector2(center.X + length, center.Y), 1.5f);
        canvas.DrawLine(Vector2(center.X, center.Y - length), Vector2(center.X, center.Y + length), 1.5f);
    }

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

    RenderEnhancedGoalBlocker(canvas, camera, frustum, ball, topLeft, topRight, bottomLeft, bottomRight);
}