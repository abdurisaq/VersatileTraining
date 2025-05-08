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
    // Get ball position for occlusion checking
    Vector ballLocation = ball.GetLocation();
    Vector2 ballScreenPos = canvas.Project(ballLocation);
    float ballRadius = Distance(ballScreenPos, canvas.Project(ballLocation + Vector(92.75f, 0, 0)));

    // Project all points to 2D for rendering
    Vector2 tl2d = canvas.Project(topLeft);
    Vector2 tr2d = canvas.Project(topRight);
    Vector2 bl2d = canvas.Project(bottomLeft);
    Vector2 br2d = canvas.Project(bottomRight);

    // Create gradient effect from top to bottom
    LinearColor topColor = { 0, 255, 0, 230 };    // Green with slight transparency at top
    LinearColor bottomColor = { 255, 128, 0, 180 }; // Orange with more transparency at bottom

    // Draw the edges with improved visual style
    canvas.SetColor(topColor);
    DrawLineClippedByCircle(canvas, tl2d, tr2d, ballScreenPos, ballRadius, topLeft, topRight, camera, frustum);

    // Gradient for right edge
    LinearColor rightColor = LerpColor(topColor, bottomColor, 0.33f);
    canvas.SetColor(rightColor);
    DrawLineClippedByCircle(canvas, tr2d, br2d, ballScreenPos, ballRadius, topRight, bottomRight, camera, frustum);

    // Bottom edge
    canvas.SetColor(bottomColor);
    DrawLineClippedByCircle(canvas, br2d, bl2d, ballScreenPos, ballRadius, bottomRight, bottomLeft, camera, frustum);

    // Gradient for left edge
    LinearColor leftColor = LerpColor(bottomColor, topColor, 0.33f);
    canvas.SetColor(leftColor);
    DrawLineClippedByCircle(canvas, bl2d, tl2d, ballScreenPos, ballRadius, bottomLeft, topLeft, camera, frustum);

    // Add interior grid pattern to make it more visible
    DrawGoalBlockerGrid(canvas, camera, frustum, ball, topLeft, topRight, bottomLeft, bottomRight);
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
    // Get ball data for occlusion
    Vector ballLocation = ball.GetLocation();
    Vector2 ballScreenPos = canvas.Project(ballLocation);
    float ballRadius = Distance(ballScreenPos, canvas.Project(ballLocation + Vector(92.75f, 0, 0)));

    // Number of grid lines
    const int gridLines = 4;

    // Calculate size of goal blocker
    float width = Distance(Vector2(topLeft.X, topLeft.Z), Vector2(topRight.X, topRight.Z));
    float height = Distance(Vector2(topLeft.X, topLeft.Z), Vector2(bottomLeft.X, bottomLeft.Z));

    // Grid lines in X direction (vertical lines)
    for (int i = 1; i < gridLines; i++) {
        float t = static_cast<float>(i) / gridLines;
        Vector topPoint = LerpVector(topLeft, topRight, t);
        Vector bottomPoint = LerpVector(bottomLeft, bottomRight, t);

        // Use a more transparent color for grid lines
        LinearColor gridColor = { 255, 255, 255, 100 };
        canvas.SetColor(gridColor);

        Vector2 topPoint2D = canvas.Project(topPoint);
        Vector2 bottomPoint2D = canvas.Project(bottomPoint);

        DrawLineClippedByCircle(canvas, topPoint2D, bottomPoint2D, ballScreenPos, ballRadius,
            topPoint, bottomPoint, camera, frustum);
    }

    // Grid lines in Z direction (horizontal lines)
    for (int i = 1; i < gridLines; i++) {
        float t = static_cast<float>(i) / gridLines;
        Vector leftPoint = LerpVector(topLeft, bottomLeft, t);
        Vector rightPoint = LerpVector(topRight, bottomRight, t);

        // Use a more transparent color for grid lines
        LinearColor gridColor = { 255, 255, 255, 100 };
        canvas.SetColor(gridColor);

        Vector2 leftPoint2D = canvas.Project(leftPoint);
        Vector2 rightPoint2D = canvas.Project(rightPoint);

        DrawLineClippedByCircle(canvas, leftPoint2D, rightPoint2D, ballScreenPos, ballRadius,
            leftPoint, rightPoint, camera, frustum);
    }
}

// Helper function to linearly interpolate between vectors
Vector VersatileTraining::LerpVector(const Vector& a, const Vector& b, float t) {
    return Vector(
        a.X + (b.X - a.X) * t,
        a.Y + (b.Y - a.Y) * t,
        a.Z + (b.Z - a.Z) * t
    );
}

// Helper function to interpolate between colors
LinearColor VersatileTraining::LerpColor(const LinearColor& a, const LinearColor& b, float t) {
    return LinearColor(
        a.R + (b.R - a.R) * t,
        a.G + (b.G - a.G) * t,
        a.B + (b.B - a.B) * t,
        a.A + (b.A - a.A) * t
    );
}