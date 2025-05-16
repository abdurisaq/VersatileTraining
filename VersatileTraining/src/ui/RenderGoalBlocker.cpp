#include "pch.h"
#include "src/core/versatileTraining.h"



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