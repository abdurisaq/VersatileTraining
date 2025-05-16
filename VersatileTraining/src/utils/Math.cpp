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
float DotV(const Vector& a, const Vector& b) {
    return a.X * b.X + a.Y * b.Y + a.Z * b.Z;
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
float Distance(Vector a, Vector b) {
	float dx = b.X - a.X;
	float dy = b.Y - a.Y;
	float dz = b.Z - a.Z;
	return sqrt(dx * dx + dy * dy + dz * dz);
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

	Vector result;
	result.X = nx * magnitude;
	result.Y = ny * magnitude;
	result.Z = nz * magnitude;

	return result;

}



void DrawLineClippedByCircles(
	CanvasWrapper& canvas,
	const Vector& p_start_3D,
	const Vector& p_end_3D,
	const std::vector<ClippingCircle>& circles_to_clip, 
	CameraWrapper& cam,
	RT::Frustum& frust,
	float thickness
) {
	std::vector<std::pair<float, float>> visible_segments;
	visible_segments.push_back({ 0.0f, 1.0f }); 

	Vector2 p_start_2D = canvas.Project(p_start_3D);
	Vector2 p_end_2D = canvas.Project(p_end_3D);
	Vector2 ab_2D = p_end_2D - p_start_2D;
	float line_len_sq_2D = Dot(ab_2D, ab_2D);

	if (line_len_sq_2D < 1e-6f) { 
		bool point_visible = true;
		for (const auto& circle : circles_to_clip) {
			if (Distance(p_start_2D, circle.center) < circle.radius) {
				point_visible = false;
				break;
			}
		}
		if (point_visible) {
			RT::Line line(p_start_3D, p_end_3D, RT::GetVisualDistance(canvas, frust, cam, p_start_3D) * 1.5f);
			line.thickness = thickness;
			line.DrawWithinFrustum(canvas, frust);
		}
		return;
	}

	for (const auto& circle : circles_to_clip) {
		if (visible_segments.empty()) break; 

		std::vector<std::pair<float, float>> next_visible_segments_for_this_circle;
		Vector2 ac_2D = p_start_2D - circle.center;

		float A_quad = line_len_sq_2D;
		float B_quad = 2 * Dot(ac_2D, ab_2D);
		float C_quad = Dot(ac_2D, ac_2D) - circle.radius * circle.radius;
		float discriminant = B_quad * B_quad - 4 * A_quad * C_quad;

		float t_circle_inter_1 = -1.0f, t_circle_inter_2 = -1.0f;

		if (discriminant >= 0.0f) {
			float sqrtDisc = std::sqrt(discriminant);
			float rcp2A = 1.0f / (2.0f * A_quad);
			t_circle_inter_1 = (-B_quad - sqrtDisc) * rcp2A;
			t_circle_inter_2 = (-B_quad + sqrtDisc) * rcp2A;
			if (t_circle_inter_1 > t_circle_inter_2) std::swap(t_circle_inter_1, t_circle_inter_2);
		}

		for (const auto& current_segment_t_pair : visible_segments) {
			float seg_t_start = current_segment_t_pair.first;
			float seg_t_end = current_segment_t_pair.second;

			if (seg_t_start >= seg_t_end) continue;

			if (discriminant < 0.0f) { 
				Vector2 mid_point_seg_2D = p_start_2D + ab_2D * ((seg_t_start + seg_t_end) / 2.0f);
				if (Distance(mid_point_seg_2D, circle.center) > circle.radius) {
					next_visible_segments_for_this_circle.push_back(current_segment_t_pair);
				}
			}
			else {
				
				float end1 = min(seg_t_end, t_circle_inter_1);
				if (end1 > seg_t_start) {
					next_visible_segments_for_this_circle.push_back({ seg_t_start, end1 });
				}

				
				float start2 = max(seg_t_start, t_circle_inter_2);
				if (seg_t_end > start2) {
					next_visible_segments_for_this_circle.push_back({ start2, seg_t_end });
				}
			}
		}
		visible_segments = next_visible_segments_for_this_circle;
	}

	Vector v3D_dir = p_end_3D - p_start_3D;
	for (const auto& seg_t_pair : visible_segments) {
		if (seg_t_pair.second > seg_t_pair.first + 1e-4f) { // Ensure valid segment 
			Vector s = p_start_3D + v3D_dir * seg_t_pair.first;
			Vector e = p_start_3D + v3D_dir * seg_t_pair.second;
			RT::Line line(s, e, RT::GetVisualDistance(canvas, frust, cam, s) * 1.5f);
			line.thickness = thickness;
			line.DrawWithinFrustum(canvas, frust);
		}
	}
}