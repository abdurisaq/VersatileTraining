#include "pch.h"
#include "src/core/VersatileTraining.h"
void VersatileTraining::Render(CanvasWrapper canvas) {
	if (editingVariances) {

		LinearColor colors;
		colors.R = 255;
		colors.G = 255;
		colors.B = 0;
		colors.A = 255;
		canvas.SetColor(colors);

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

		canvas.DrawString("Starting Velocity: " + std::to_string(currentShotState.startingVelocity), 2.0, 2.0, false);
	}
	else if (editingGoalBlocker) {
		CameraWrapper cam = gameWrapper->GetCamera();
		if (cam.IsNull()) return;

		Vector camLoc = cam.GetLocation();
		Vector forward = GetForwardVectorFromRotator(cam.GetRotation());
		RT::Frustum frust{ canvas, cam };

		Vector projectedPoint;
		if (ProjectToPlaneY(camLoc, forward, backWall, projectedPoint)) {
			// Draw projected point as a small cross
			Vector2 screenPoint = canvas.Project(projectedPoint);
			float crossSize = 5.0f;
			canvas.SetColor(255, 255, 255, 255);
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
				//eventaully add logic for second middle mouse press to keep the goal blocker seperate from mouse
			}
			else if (currentShotState.goalAnchors.first && !currentShotState.goalAnchors.second) {

				currentShotState.goalBlocker.second = projectedPoint;
				//LOG("updating second anchor point : X {}, Z {}", goalBlockerPos.second.X, goalBlockerPos.second.Z);
				rectangleMade = true;

			}if (currentShotState.goalAnchors.first && currentShotState.goalAnchors.second) {
				rectangleMade = true;
			}


			// If we have both points, draw rectangle on the goal plane
			if (rectangleMade) {
				Vector topLeft(max(currentShotState.goalBlocker.first.X, currentShotState.goalBlocker.second.X), backWall, max(currentShotState.goalBlocker.first.Z, currentShotState.goalBlocker.second.Z));
				Vector topRight(min(currentShotState.goalBlocker.first.X, currentShotState.goalBlocker.second.X), backWall, max(currentShotState.goalBlocker.first.Z, currentShotState.goalBlocker.second.Z));
				Vector bottomLeft(max(currentShotState.goalBlocker.first.X, currentShotState.goalBlocker.second.X), backWall, min(currentShotState.goalBlocker.first.Z, currentShotState.goalBlocker.second.Z));
				Vector bottomRight(min(currentShotState.goalBlocker.first.X, currentShotState.goalBlocker.second.X), backWall, min(currentShotState.goalBlocker.first.Z, currentShotState.goalBlocker.second.Z));

				canvas.SetColor(0, 255, 0, 255); // Green

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

		// Optional: Draw center screen cursor
		Vector2 screenSize = canvas.GetSize();
		Vector2 center(screenSize.X / 2, screenSize.Y / 2);
		canvas.SetColor(255, 255, 255, 255);
		float length = 10.0f;
		canvas.DrawLine(Vector2(center.X - length, center.Y), Vector2(center.X + length, center.Y), 1.5f);
		canvas.DrawLine(Vector2(center.X, center.Y - length), Vector2(center.X, center.Y + length), 1.5f);


	}

	//if (!gameWrapper->IsInCustomTraining()) return;

	if (!(currentShotState.goalAnchors.first && currentShotState.goalAnchors.second)) return;


	CameraWrapper camera = gameWrapper->GetCamera();
	if (camera.IsNull()) return;

	RT::Frustum frustum{ canvas, camera };
	ServerWrapper gameState = gameWrapper->GetCurrentGameState();
	BallWrapper ball = gameState.GetBall();
	if (ball.IsNull()) return;

	Vector2 ballScreenPos = canvas.Project(ball.GetLocation());
	//float ballRadius = canvas.Project(ball.GetLocation() + Vector(92.75f, 0, 0)).Dist(ballScreenPos); // 92.75 uu = ball radius in RL
	float ballRadius = Distance(ballScreenPos, canvas.Project(ball.GetLocation() + Vector(92.75f, 0, 0)));
	// Goal blocker corner definitions
	Vector topLeft(max(currentShotState.goalBlocker.first.X, currentShotState.goalBlocker.second.X), backWall, max(currentShotState.goalBlocker.first.Z, currentShotState.goalBlocker.second.Z));
	Vector topRight(min(currentShotState.goalBlocker.first.X, currentShotState.goalBlocker.second.X), backWall, max(currentShotState.goalBlocker.first.Z, currentShotState.goalBlocker.second.Z));
	Vector bottomLeft(max(currentShotState.goalBlocker.first.X, currentShotState.goalBlocker.second.X), backWall, min(currentShotState.goalBlocker.first.Z, currentShotState.goalBlocker.second.Z));
	Vector bottomRight(min(currentShotState.goalBlocker.first.X, currentShotState.goalBlocker.second.X), backWall, min(currentShotState.goalBlocker.first.Z, currentShotState.goalBlocker.second.Z));

	// Project all points to 2D
	Vector2 tl2d = canvas.Project(topLeft);
	Vector2 tr2d = canvas.Project(topRight);
	Vector2 bl2d = canvas.Project(bottomLeft);
	Vector2 br2d = canvas.Project(bottomRight);

	canvas.SetColor(0, 255, 0, 255); // Green

	DrawLineClippedByCircle(canvas, tl2d, tr2d, ballScreenPos, ballRadius, topLeft, topRight, camera, frustum);
	DrawLineClippedByCircle(canvas, tr2d, br2d, ballScreenPos, ballRadius, topRight, bottomRight, camera, frustum);
	DrawLineClippedByCircle(canvas, br2d, bl2d, ballScreenPos, ballRadius, bottomRight, bottomLeft, camera, frustum);
	DrawLineClippedByCircle(canvas, bl2d, tl2d, ballScreenPos, ballRadius, bottomLeft, topLeft, camera, frustum);

}
