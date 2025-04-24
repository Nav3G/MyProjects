#pragma once

#include <vector>
#include <array>
#include <functional>
#include "Vec4.h"

namespace GeometryUtils
{
	// A polygon in clip space
	using Vec4Poly = std::vector<Vec4>;
	// A triangle composed of three clip-space vertices
	using Tri4 = std::array<Vec4, 3>;
	// A function that returns the signed distance (>=0 is inside)
	using PlaneFn = std::function<float(const Vec4&)>;

	//Compute intersection point of segment AB with plane defined by f:
	//f(A) >= 0 is inside
	//Returns the point I such that f(I) == 0.
	Vec4 intersectPlane(const Vec4& A, const Vec4& B, PlaneFn f);

	
	// Clip a convex polygon against a single plane.
	// Keeps vertices v where f(v) >= 0.
	// Returns the clipped polygon (may have 0 - n vertices).
	Vec4Poly clipPolygon(const Vec4Poly& poly, PlaneFn f);

	
	// Triangulate a convex polygon into a fan of triangles around poly[0].
	// Input: poly.size() >= 3, convex order.
	// Output: list of Tri4 where each Tri4 is {poly[0], poly[i], poly[i+1]}.
	std::vector<Tri4> triangulateFan(const Vec4Poly& poly);

	// Clip‐space plane tests (>=0 means inside):
	float planeLeft(const Vec4& v);		// v.x + v.w
	float planeRight(const Vec4& v);	// -v.x + v.w
	float planeBottom(const Vec4& v);	// v.y + v.w
	float planeTop(const Vec4& v);		// -v.y + v.w
	float planeNear(const Vec4& v);		// v.z + v.w
	float planeFar(const Vec4& v);		// -v.z + v.w
};

