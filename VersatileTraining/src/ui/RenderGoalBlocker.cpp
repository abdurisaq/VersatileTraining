#include "pch.h"
#include "src/core/versatileTraining.h"


float GOAL_MIN_X_WORLD = -910; 
float GOAL_MAX_X_WORLD = 910;
float GOAL_MIN_Z_WORLD = -20.0f;    
float GOAL_MAX_Z_WORLD = 660.0f;    

void VersatileTraining::RenderEnhancedGoalBlocker(
    CanvasWrapper& canvas,
    CameraWrapper& camera,
    RT::Frustum& frustum,
    BallWrapper& ball,
    const Vector& topLeft,
    const Vector& topRight,
    const Vector& bottomLeft,
    const Vector& bottomRight)
{
    std::vector<ClippingCircle> clipping_circles;
    Quat cameraOrientation = RotatorToQuat(camera.GetRotation());
    Vector cameraRightVector = RotateVectorWithQuat(Vector(0, 1, 0), cameraOrientation); 

    if (!ball.IsNull()) {
        Vector ballLocation = ball.GetLocation();
        Vector2 ballScreenPos = canvas.Project(ballLocation);
        float ballWorldRadius = ball.GetRadius();
        
        Vector ballEdgeWorld = ballLocation + cameraRightVector * ballWorldRadius;
        Vector2 ballEdgeScreen = canvas.Project(ballEdgeWorld);
        float ballScreenSpaceRadius = Distance(ballScreenPos, ballEdgeScreen);
        clipping_circles.push_back({ ballScreenPos, max(0.f, ballScreenSpaceRadius) });
    }

   
    CarWrapper car = gameWrapper->GetLocalCar();
    if (!car.IsNull()) {
        Vector carWorldPos = car.GetLocation();
        Vector2 carScreenPos = canvas.Project(carWorldPos);
        Vector carExtent = car.GetLocalCollisionExtent();

        
        float carWorldRadiusApproximation = max(carExtent.X, carExtent.Y, carExtent.Z); 
        

        
        Vector carEdgeWorldForClip = carWorldPos + cameraRightVector * carWorldRadiusApproximation;
        Vector2 carEdgeScreenForClip = canvas.Project(carEdgeWorldForClip);
        float carScreenSpaceRadius = Distance(carScreenPos, carEdgeScreenForClip);
        if (carScreenSpaceRadius > 1.0f) {
            clipping_circles.push_back({ carScreenPos, max(0.f, carScreenSpaceRadius) });
        }

       
    }

    canvas.SetColor(goalBlockerOutlineColor);

    DrawLineClippedByCircles(canvas, topLeft, topRight, clipping_circles, camera, frustum, goalBlockerOutlineThickness);
    DrawLineClippedByCircles(canvas, topRight, bottomRight, clipping_circles, camera, frustum, goalBlockerOutlineThickness);
    DrawLineClippedByCircles(canvas, bottomRight, bottomLeft, clipping_circles, camera, frustum, goalBlockerOutlineThickness);
    DrawLineClippedByCircles(canvas, bottomLeft, topLeft, clipping_circles, camera, frustum, goalBlockerOutlineThickness);

    DrawGoalBlockerGrid(canvas, camera, frustum, ball, car, topLeft, topRight, bottomLeft, bottomRight);
}

void VersatileTraining::DrawGoalBlockerGrid(
    CanvasWrapper& canvas,
    CameraWrapper& camera,
    RT::Frustum& frustum,
    BallWrapper& ball,
    CarWrapper& car,
    const Vector& topLeft,
    const Vector& topRight,
    const Vector& bottomLeft,
    const Vector& bottomRight)
{
    const int numGridLines = goalBlockerGridLines;
    if (numGridLines <= 0) return;

    std::vector<ClippingCircle> clipping_circles_grid;
    Quat cameraOrientation = RotatorToQuat(camera.GetRotation());
    Vector cameraRightVector = RotateVectorWithQuat(Vector(0, 1, 0), cameraOrientation); 

    if (!ball.IsNull()) {
        Vector ballLocation = ball.GetLocation();
        Vector2 ballScreenPos = canvas.Project(ballLocation);
        float ballWorldRadius = ball.GetRadius();
        
        Vector ballEdgeWorld = ballLocation + cameraRightVector * ballWorldRadius;
        Vector2 ballEdgeScreen = canvas.Project(ballEdgeWorld);
        float ballScreenSpaceRadius = Distance(ballScreenPos, ballEdgeScreen);
        clipping_circles_grid.push_back({ ballScreenPos, max(0.f, ballScreenSpaceRadius) });
    }

    if (!car.IsNull()) {
        Vector carWorldPos = car.GetLocation();
        Vector2 carScreenPos = canvas.Project(carWorldPos);
        Vector carExtent = car.GetLocalCollisionExtent();

        float carWorldRadiusApproximation = carExtent.X;
        if (carExtent.Y > carWorldRadiusApproximation) {
            carWorldRadiusApproximation = carExtent.Y;
        }
        if (carExtent.Z > carWorldRadiusApproximation) {
            carWorldRadiusApproximation = carExtent.Z;
        }

        
        Vector carEdgeWorld = carWorldPos + cameraRightVector * carWorldRadiusApproximation;
        Vector2 carEdgeScreen = canvas.Project(carEdgeWorld);
        float carScreenSpaceRadius = Distance(carScreenPos, carEdgeScreen);
        if (carScreenSpaceRadius > 1.0f) {
            clipping_circles_grid.push_back({ carScreenPos, max(0.f, carScreenSpaceRadius) });
        }
    }

    canvas.SetColor(goalBlockerGridColor);

    for (int i = 1; i < numGridLines; i++) {
        float t = static_cast<float>(i) / numGridLines;
        Vector topPoint = LerpVector(topLeft, topRight, t);
        Vector bottomPoint = LerpVector(bottomLeft, bottomRight, t);
        DrawLineClippedByCircles(canvas, topPoint, bottomPoint, clipping_circles_grid, camera, frustum, goalBlockerGridThickness);
    }

    for (int i = 1; i < numGridLines; i++) {
        float t = static_cast<float>(i) / numGridLines;
        Vector leftPoint = LerpVector(topLeft, bottomLeft, t);
        Vector rightPoint = LerpVector(topRight, bottomRight, t);
        DrawLineClippedByCircles(canvas, leftPoint, rightPoint, clipping_circles_grid, camera, frustum, goalBlockerGridThickness);
    }
}


void VersatileTraining::RenderInvertedGoalBlocker_OutlinedAndGridded(
    CanvasWrapper& canvas,
    CameraWrapper& camera,
    RT::Frustum& frustum,
    BallWrapper& ball,
    const Vector& openingTopLeft,
    const Vector& openingTopRight,
    const Vector& openingBottomLeft,
    const Vector& openingBottomRight)
{
    float goalY = openingTopLeft.Y; // Y-coordinate of the goal plane

    // --- 1. Calculate Clipping Circles (same as in RenderEnhancedGoalBlocker) ---
    std::vector<ClippingCircle> clipping_circles;
    Quat cameraOrientation = RotatorToQuat(camera.GetRotation());
    Vector cameraRightVector = RotateVectorWithQuat(Vector(0, 1, 0), cameraOrientation);

    if (!ball.IsNull()) {
        Vector ballLocation = ball.GetLocation();
        Vector2 ballScreenPos = canvas.Project(ballLocation);
        float ballWorldRadius = ball.GetRadius();
        Vector ballEdgeWorld = ballLocation + cameraRightVector * ballWorldRadius;
        Vector2 ballEdgeScreen = canvas.Project(ballEdgeWorld);
        float ballScreenSpaceRadius = Distance(ballScreenPos, ballEdgeScreen);
        clipping_circles.push_back({ ballScreenPos, max(0.f, ballScreenSpaceRadius) });
    }

    CarWrapper car = gameWrapper->GetLocalCar();
    if (!car.IsNull()) {
        Vector carWorldPos = car.GetLocation();
        Vector2 carScreenPos = canvas.Project(carWorldPos);
        Vector carExtent = car.GetLocalCollisionExtent();
        float carWorldRadiusApproximation = max(carExtent.X, carExtent.Y, carExtent.Z);
        Vector carEdgeWorldForClip = carWorldPos + cameraRightVector * carWorldRadiusApproximation;
        Vector2 carEdgeScreenForClip = canvas.Project(carEdgeWorldForClip);
        float carScreenSpaceRadius = Distance(carScreenPos, carEdgeScreenForClip);
        if (carScreenSpaceRadius > 1.0f) { // Avoid tiny/invalid circles
            clipping_circles.push_back({ carScreenPos, max(0.f, carScreenSpaceRadius) });
        }
    }

    // --- 2. Draw Outer Goal Outline ---
    canvas.SetColor(goalBlockerOutlineColor);
    Vector goal_TL = {GOAL_MAX_X_WORLD, goalY, GOAL_MAX_Z_WORLD};
    Vector goal_TR = {GOAL_MIN_X_WORLD, goalY, GOAL_MAX_Z_WORLD};
    Vector goal_BL = {GOAL_MAX_X_WORLD, goalY, GOAL_MIN_Z_WORLD};
    Vector goal_BR = {GOAL_MIN_X_WORLD, goalY, GOAL_MIN_Z_WORLD};

    DrawLineClippedByCircles(canvas, goal_TL, goal_TR, clipping_circles, camera, frustum, goalBlockerOutlineThickness);
    DrawLineClippedByCircles(canvas, goal_TR, goal_BR, clipping_circles, camera, frustum, goalBlockerOutlineThickness);
    DrawLineClippedByCircles(canvas, goal_BR, goal_BL, clipping_circles, camera, frustum, goalBlockerOutlineThickness);
    DrawLineClippedByCircles(canvas, goal_BL, goal_TL, clipping_circles, camera, frustum, goalBlockerOutlineThickness);

    // --- 3. Draw Inner Opening Outline ---
    // (openingTopLeft, openingTopRight, openingBottomLeft, openingBottomRight are already the correct corners)
    DrawLineClippedByCircles(canvas, openingTopLeft, openingTopRight, clipping_circles, camera, frustum, goalBlockerOutlineThickness);
    DrawLineClippedByCircles(canvas, openingTopRight, openingBottomRight, clipping_circles, camera, frustum, goalBlockerOutlineThickness);
    DrawLineClippedByCircles(canvas, openingBottomRight, openingBottomLeft, clipping_circles, camera, frustum, goalBlockerOutlineThickness);
    DrawLineClippedByCircles(canvas, openingBottomLeft, openingTopLeft, clipping_circles, camera, frustum, goalBlockerOutlineThickness);

    // --- 4. Draw Grid Lines in the "Blocked" Area ---
    canvas.SetColor(goalBlockerGridColor);
    const int numGridLines = goalBlockerGridLines;
    if (numGridLines <= 0) return;

    // Helper values for opening boundaries
    float openingMaxX = openingTopLeft.X;
    float openingMinX = openingTopRight.X;
    float openingMaxZ = openingTopLeft.Z;
    float openingMinZ = openingBottomLeft.Z;

    // Horizontal Grid Lines
    for (int i = 1; i < numGridLines; ++i) {
        float t = static_cast<float>(i) / numGridLines;
        float currentZ = GOAL_MIN_Z_WORLD + (GOAL_MAX_Z_WORLD - GOAL_MIN_Z_WORLD) * t;

        // If currentZ is within the Z-range of the opening
        if (currentZ > openingMinZ && currentZ < openingMaxZ) {
            // Draw segment from left goal edge to left opening edge
            if (GOAL_MAX_X_WORLD > openingMaxX) { // Check if there's space to the left of opening
                 DrawLineClippedByCircles(canvas,
                    {GOAL_MAX_X_WORLD, goalY, currentZ},
                    {openingMaxX, goalY, currentZ},
                    clipping_circles, camera, frustum, goalBlockerGridThickness);
            }
            // Draw segment from right opening edge to right goal edge
            if (openingMinX > GOAL_MIN_X_WORLD) { // Check if there's space to the right of opening
                DrawLineClippedByCircles(canvas,
                    {openingMinX, goalY, currentZ},
                    {GOAL_MIN_X_WORLD, goalY, currentZ},
                    clipping_circles, camera, frustum, goalBlockerGridThickness);
            }
        } else { // currentZ is outside the opening's Z-range, draw full width
            DrawLineClippedByCircles(canvas,
                {GOAL_MAX_X_WORLD, goalY, currentZ},
                {GOAL_MIN_X_WORLD, goalY, currentZ},
                clipping_circles, camera, frustum, goalBlockerGridThickness);
        }
    }

    // Vertical Grid Lines
    for (int i = 1; i < numGridLines; ++i) {
        float t = static_cast<float>(i) / numGridLines;
        // Iterate from Min X to Max X for vertical lines to match visual expectation
        float currentX = GOAL_MIN_X_WORLD + (GOAL_MAX_X_WORLD - GOAL_MIN_X_WORLD) * t;

        // If currentX is within the X-range of the opening
        if (currentX > openingMinX && currentX < openingMaxX) {
            // Draw segment from top goal edge to top opening edge
            if (GOAL_MAX_Z_WORLD > openingMaxZ) { // Check if there's space above opening
                DrawLineClippedByCircles(canvas,
                    {currentX, goalY, GOAL_MAX_Z_WORLD},
                    {currentX, goalY, openingMaxZ},
                    clipping_circles, camera, frustum, goalBlockerGridThickness);
            }
            // Draw segment from bottom opening edge to bottom goal edge
            if (openingMinZ > GOAL_MIN_Z_WORLD) { // Check if there's space below opening
                DrawLineClippedByCircles(canvas,
                    {currentX, goalY, openingMinZ},
                    {currentX, goalY, GOAL_MIN_Z_WORLD},
                    clipping_circles, camera, frustum, goalBlockerGridThickness);
            }
        } else { // currentX is outside the opening's X-range, draw full height
            DrawLineClippedByCircles(canvas,
                {currentX, goalY, GOAL_MAX_Z_WORLD},
                {currentX, goalY, GOAL_MIN_Z_WORLD},
                clipping_circles, camera, frustum, goalBlockerGridThickness);
        }
    }
}

Vector VersatileTraining::LerpVector(const Vector& a, const Vector& b, float t) {
    return Vector(
        a.X + (b.X - a.X) * t,
        a.Y + (b.Y - a.Y) * t,
        a.Z + (b.Z - a.Z) * t
    );
}

LinearColor VersatileTraining::LerpColor(const LinearColor& a, const LinearColor& b, float t) {
    return LinearColor(
        a.R + (b.R - a.R) * t,
        a.G + (b.G - a.G) * t,
        a.B + (b.B - a.B) * t,
        a.A + (b.A - a.A) * t
    );
}


