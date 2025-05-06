#include "pch.h"
struct FVector {
	float X, Y, Z;

	FVector() : X(0), Y(0), Z(0) {}
	FVector(float x, float y, float z) : X(x), Y(y), Z(z) {}
};
struct FRotator {
	float Pitch, Yaw, Roll;

	FRotator() : Pitch(0), Yaw(0), Roll(0) {}
	FRotator(float pitch, float yaw, float roll) : Pitch(pitch), Yaw(yaw), Roll(roll) {}
};


Vector GetForwardVectorFromRotator(const Rotator& rot) {
	float yawRad = rot.Yaw * (2.0f * PI / 65536.0f);
	float pitchRad = rot.Pitch * (PI / 32768.0f);

	float cosPitch = cosf(pitchRad);

	return Vector{
		cosPitch * cosf(yawRad), // X
		cosPitch * sinf(yawRad), // Y
		sinf(pitchRad)           // Z
	};
}

bool ProjectToPlaneY(Vector origin, Vector direction, float yPlane, Vector& outHit) {
	// get point by projecting onto Y = backWall
	if (fabs(direction.Y) < 1e-6) return false; //to make sure it doesnt fuck with 0

	float t = (yPlane - origin.Y) / direction.Y;
	if (t < 0) return false; // Behind the camera

	outHit = origin + direction * t;
	return true;
}

float Dot(const Vector2& a, const Vector2& b) {
	return a.X * b.X + a.Y * b.Y;
}

void DrawLineClippedByCircle(CanvasWrapper& canvas, const Vector2& a, const Vector2& b, const Vector2& circleCenter, float radius, const Vector& a3D, const Vector& b3D, CameraWrapper cam, RT::Frustum frust) {
	Vector2 ab = b - a;
	Vector2 ac = a - circleCenter;

	float A = Dot(ab, ab);
	float B = 2 * Dot(ac, ab);
	float C = Dot(ac, ac) - radius * radius;

	float discriminant = B * B - 4 * A * C;

	// No intersection: draw full line
	if (discriminant < 0.0f) {
		RT::Line full(a3D, b3D, RT::GetVisualDistance(canvas, frust, cam, a3D) * 1.5f);
		full.DrawWithinFrustum(canvas, frust);
		return;
	}

	// Compute intersection points (in screen space as t values)
	float sqrtDisc = std::sqrt(discriminant);
	float t1 = (-B - sqrtDisc) / (2 * A);
	float t2 = (-B + sqrtDisc) / (2 * A);

	t1 = std::clamp(t1, 0.0f, 1.0f);
	t2 = std::clamp(t2, 0.0f, 1.0f);

	Vector2 p1 = a + ab * t1;
	Vector2 p2 = a + ab * t2;

	Vector v3D = b3D - a3D;
	Vector p1_3D = a3D + v3D * t1;
	Vector p2_3D = a3D + v3D * t2;

	if (t1 > 0.0f) {
		RT::Line line1(a3D, p1_3D, RT::GetVisualDistance(canvas, frust, cam, a3D) * 1.5f);
		line1.DrawWithinFrustum(canvas, frust);
	}
	if (t2 < 1.0f) {
		RT::Line line2(p2_3D, b3D, RT::GetVisualDistance(canvas, frust, cam, b3D) * 1.5f);
		line2.DrawWithinFrustum(canvas, frust);
	}
}


float Distance(Vector2 a, Vector2 b) {
	float dx = b.X - a.X;
	float dy = b.Y - a.Y;
	return sqrt(dx * dx + dy * dy);
}

bool inRectangle(const std::pair<Vector, Vector>& goalBlockerPos, const Vector& ballLoc) {
	float minX = min(goalBlockerPos.first.X, goalBlockerPos.second.X);
	float maxX = max(goalBlockerPos.first.X, goalBlockerPos.second.X);

	float minZ = min(goalBlockerPos.first.Z, goalBlockerPos.second.Z);
	float maxZ = max(goalBlockerPos.first.Z, goalBlockerPos.second.Z);

	// Y is usually constant (the back wall), so we just care about X and Z for 2D plane
	return (ballLoc.X >= minX && ballLoc.X <= maxX &&
		ballLoc.Z >= minZ && ballLoc.Z <= maxZ);
}


Vector convertRotationAndMagnitudeToVector(const Rotator& rot, float magnitude) {

	float pitch_deg = rot.Pitch * (90.0f / 16384.0f);
	float yaw_deg = rot.Yaw * (360.0f / 65536.0f);

	// Convert degrees to radians
	float pitch_rad = pitch_deg * (PI / 180.0f);
	float yaw_rad = yaw_deg * (PI / 180.0f);

	// Compute normalized direction vector
	float nx = std::cos(pitch_rad) * std::cos(yaw_rad);
	float ny = std::cos(pitch_rad) * std::sin(yaw_rad);
	float nz = std::sin(pitch_rad);

	// Scale by magnitude
	Vector result;
	result.X = nx * magnitude;
	result.Y = ny * magnitude;
	result.Z = nz * magnitude;

	return result;

}