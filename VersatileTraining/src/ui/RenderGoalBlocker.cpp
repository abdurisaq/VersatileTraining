#include "pch.h"
#include "src/core/versatileTraining.h"


void DrawLineClippedWithThickness(CanvasWrapper& canvas, const Vector2& a, const Vector2& b,
    const Vector2& circleCenter, float radius, const Vector& a3D, const Vector& b3D,
    CameraWrapper cam, RT::Frustum frust, float thickness);


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
    float ballRadius = Distance(ballScreenPos, canvas.Project(ballLocation + Vector(92.75f, 0, 0)));

    Vector2 tl2d = canvas.Project(topLeft);
    Vector2 tr2d = canvas.Project(topRight);
    Vector2 bl2d = canvas.Project(bottomLeft);
    Vector2 br2d = canvas.Project(bottomRight);

    canvas.SetColor(goalBlockerOutlineColor);

    RT::Line lineTop(topLeft, topRight, RT::GetVisualDistance(canvas, frustum, camera, topLeft) * 1.5f);
    lineTop.thickness = goalBlockerOutlineThickness;
    lineTop.DrawWithinFrustum(canvas, frustum);

    RT::Line lineRight(topRight, bottomRight, RT::GetVisualDistance(canvas, frustum, camera, topRight) * 1.5f);
    lineRight.thickness = goalBlockerOutlineThickness;
    lineRight.DrawWithinFrustum(canvas, frustum);

    
    RT::Line lineBottom(bottomRight, bottomLeft, RT::GetVisualDistance(canvas, frustum, camera, bottomRight) * 1.5f);
    lineBottom.thickness = goalBlockerOutlineThickness;
    lineBottom.DrawWithinFrustum(canvas, frustum);

    // Left edge with thickness
    RT::Line lineLeft(bottomLeft, topLeft, RT::GetVisualDistance(canvas, frustum, camera, bottomLeft) * 1.5f);
    lineLeft.thickness = goalBlockerOutlineThickness;
    lineLeft.DrawWithinFrustum(canvas, frustum);

    
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
    const int gridLines = goalBlockerGridLines;

 
    canvas.SetColor(goalBlockerGridColor);

    
    for (int i = 1; i < gridLines; i++) {
        float t = static_cast<float>(i) / gridLines;
        Vector topPoint = LerpVector(topLeft, topRight, t);
        Vector bottomPoint = LerpVector(bottomLeft, bottomRight, t);

        RT::Line gridLine(topPoint, bottomPoint, RT::GetVisualDistance(canvas, frustum, camera, topPoint) * 1.5f);
        gridLine.thickness = goalBlockerGridThickness;
        gridLine.DrawWithinFrustum(canvas, frustum);
    }

    for (int i = 1; i < gridLines; i++) {
        float t = static_cast<float>(i) / gridLines;
        Vector leftPoint = LerpVector(topLeft, bottomLeft, t);
        Vector rightPoint = LerpVector(topRight, bottomRight, t);

        RT::Line gridLine(leftPoint, rightPoint, RT::GetVisualDistance(canvas, frustum, camera, leftPoint) * 1.5f);
        gridLine.thickness = goalBlockerGridThickness;
        gridLine.DrawWithinFrustum(canvas, frustum);
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