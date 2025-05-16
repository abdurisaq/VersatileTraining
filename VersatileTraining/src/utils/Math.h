#pragma once
#include "pch.h"



Vector GetForwardVectorFromRotator(const Rotator& rot);

bool ProjectToPlaneY(Vector origin, Vector direction, float yPlane, Vector& outHit);

float Dot(const Vector2& a, const Vector2& b);

float DotV(const Vector& a, const Vector& b);
void DrawLineClippedByCircle(CanvasWrapper& canvas, const Vector2& a, const Vector2& b, const Vector2& circleCenter, float radius, const Vector& a3D, const Vector& b3D, CameraWrapper cam, RT::Frustum frust);


float Distance(Vector2 a, Vector2 b);
float Distance(Vector a , Vector b);

bool inRectangle(const std::pair<Vector, Vector>& goalBlockerPos, const Vector& ballLoc);


Vector convertRotationAndMagnitudeToVector(const Rotator& rot, float magnitude);



struct ClippingCircle {
    Vector2 center;
    float radius;
};

void DrawLineClippedByCircles(
    CanvasWrapper& canvas,
    const Vector& p_start_3D,
    const Vector& p_end_3D,
    const std::vector<ClippingCircle>& circles,
    CameraWrapper& cam,
    RT::Frustum& frust,
    float thickness
);