#include "GeometryUtils.h"
#include <stdexcept>
#include <iostream>
#include <algorithm>

namespace GeometryUtils
{
	// Triangulate a convex polygon into a fan of triangles around poly[0].
	// Input: poly.size() >= 3, convex order.
	// Output: list of Tri4 where each Tri4 is {poly[0], poly[i], poly[i+1]}.
	std::vector<Tri4> triangulateFan(const Vec4Poly& poly)
	{
		if (poly.size() < 3)
			throw std::invalid_argument("Need at least a triangle");

		std::vector<Tri4> tris;
		Vec4 root = poly[0];

		for (int i = 1; i < poly.size() - 1; i++)
		{
			Vec4 v1 = poly[i];
			Vec4 v2 = poly[i + 1];

			tris.push_back({ root, v1, v2 });
		}

		return tris;
	}

	// Compute intersection point of segment AB with plane defined by f:
	// f(A) >= 0 is inside
	// Then given the line I=A+t(B-A) i.e. the edge defined by A and B, we 
	// want the value t such that our point is on the plane. We solve 
	// f(A + t(B-A)) = f(A) + t(f(B) - f(A)) = 0 ==> t = f(A)/(f(A) - f(B)).
	// Then we just define that point via a vec4.
	// Returns the point I such that f(I) == 0.
	Vec4 intersectPlane(const Vec4& A, const Vec4& B, PlaneFn f)
	{
		float dA = f(A);
		float dB = f(B);

		// Avoid division by zero: if edge is parallel or both points lie on the plane,
		// simply return A 
		if (dA == dB) return A;

		float t = dA / (dA - dB);		// fraction along edge A -> B where the plane is hit
		if (t < 0.0f) t = 0.0f;			// Clamp t
		if (t > 1.0f) t = 1.0f;
		Vec4 I = A + (B - A) * t;		// linear interpolation in Vec4 space
		return I;
	}

	// Clip a convex polygon against a single plane.
	// Keeps vertices v where f(v) >= 0.
	// Returns the clipped polygon (may have 0 - n vertices).
	Vec4Poly clipPolygon(const Vec4Poly& poly, PlaneFn f)
	{
		Vec4Poly output;
		output.reserve(poly.size() + 2);

		if (poly.empty()) return output;

		for (size_t i = 0; i < poly.size(); i++)
		{
			const Vec4 A = poly[i];
			const Vec4 B = poly[(i + 1) % poly.size()];

			float dA = f(A);
			float dB = f(B);

			const float EPS = 1e-3f;
			bool inA = (dA >= EPS);
			bool inB = (dB >= EPS);

			if (inA && inB) output.push_back(B);
			else if (inA && !inB) output.push_back(intersectPlane(A, B, f));
			else if (!inA && inB) { output.push_back(intersectPlane(A, B, f)); output.push_back(B); }
		}

		return output;
	}

	// Clip-space plane tests (>=0 means inside):
	float planeLeft(const Vec4& v) { return v.x() + v.w(); };	// v.x + v.w
	float planeRight(const Vec4& v) { return -v.x() + v.w(); };	// -v.x + v.w
	float planeBottom(const Vec4& v) { return v.y() + v.w(); };	// v.y + v.w
	float planeTop(const Vec4& v) { return -v.y() + v.w(); };	// -v.y + v.w
	float planeNear(const Vec4& v) { return v.z() + v.w(); };	// v.z + v.w
	float planeFar(const Vec4& v) { return -v.z() + v.w(); };	// -v.z + v.w
}
