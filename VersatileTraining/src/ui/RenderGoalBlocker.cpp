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
    Vector ballLocation = ball.GetLocation();
    Vector2 ballScreenPos = canvas.Project(ballLocation);
    // Ensure ballRadius is non-negative
    float ballRadius = max(0.f, Distance(ballScreenPos, canvas.Project(ballLocation + Vector(ball.GetRadius(), 0, 0))));


    Vector2 tl2d = canvas.Project(topLeft);
    Vector2 tr2d = canvas.Project(topRight);
    Vector2 bl2d = canvas.Project(bottomLeft);
    Vector2 br2d = canvas.Project(bottomRight);

    canvas.SetColor(goalBlockerOutlineColor);

    // Top edge
    DrawLineClippedByCircle(canvas, tl2d, tr2d, ballScreenPos, ballRadius, topLeft, topRight, camera, frustum, goalBlockerOutlineThickness);

    // Right edge
    DrawLineClippedByCircle(canvas, tr2d, br2d, ballScreenPos, ballRadius, topRight, bottomRight, camera, frustum, goalBlockerOutlineThickness);

    // Bottom edge
    DrawLineClippedByCircle(canvas, br2d, bl2d, ballScreenPos, ballRadius, bottomRight, bottomLeft, camera, frustum, goalBlockerOutlineThickness);

    // Left edge
    DrawLineClippedByCircle(canvas, bl2d, tl2d, ballScreenPos, ballRadius, bottomLeft, topLeft, camera, frustum, goalBlockerOutlineThickness);

    DrawGoalBlockerGrid(canvas, camera, frustum, ball, topLeft, topRight, bottomLeft, bottomRight); // Grid will also need this treatment
}

void VersatileTraining::DrawGoalBlockerGrid(
    CanvasWrapper& canvas,
    CameraWrapper& camera,
    RT::Frustum& frustum,
    BallWrapper& ball,
    const Vector& topLeft,
    const Vector& topRight,
    const Vector& bottomLeft,
    const Vector& bottomRight)
{
    const int gridLines = goalBlockerGridLines;
    if (gridLines <= 0) return;

    Vector ballLocation = ball.GetLocation(); // Need ball info for clipping
    Vector2 ballScreenPos = canvas.Project(ballLocation);
    float ballRadius =ball.GetRadius(); // max(0.f, Distance(ballScreenPos, canvas.Project(ballLocation + Vector(ball.GetRadius(), 0, 0))))

    canvas.SetColor(goalBlockerGridColor);

    // Vertical grid lines
    for (int i = 1; i < gridLines; i++) {
        float t = static_cast<float>(i) / gridLines;
        Vector topPoint = LerpVector(topLeft, topRight, t);
        Vector bottomPoint = LerpVector(bottomLeft, bottomRight, t);

        Vector2 topPoint2D = canvas.Project(topPoint);
        Vector2 bottomPoint2D = canvas.Project(bottomPoint);
        DrawLineClippedByCircle(canvas, topPoint2D, bottomPoint2D, ballScreenPos, ballRadius, topPoint, bottomPoint, camera, frustum, goalBlockerGridThickness);
    }

    // Horizontal grid lines
    for (int i = 1; i < gridLines; i++) {
        float t = static_cast<float>(i) / gridLines;
        Vector leftPoint = LerpVector(topLeft, bottomLeft, t);
        Vector rightPoint = LerpVector(topRight, bottomRight, t);

        Vector2 leftPoint2D = canvas.Project(leftPoint);
        Vector2 rightPoint2D = canvas.Project(rightPoint);
        DrawLineClippedByCircle(canvas, leftPoint2D, rightPoint2D, ballScreenPos, ballRadius, leftPoint, rightPoint, camera, frustum, goalBlockerGridThickness);
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