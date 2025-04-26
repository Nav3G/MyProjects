#include "pipeline/RasterUtils.h"

using namespace Pipeline;

namespace RasterUtils
{
	// Bounding box type and computation
	BBox computeBBox(const ScreenTriangle3D& T,
		int w, int h)
	{
		// Compute min/max in float
		float fx0 = std::min({ T.s0.x_, T.s1.x_, T.s2.x_ });
		float fx1 = std::max({ T.s0.x_, T.s1.x_, T.s2.x_ });
		float fy0 = std::min({ T.s0.y_, T.s1.y_, T.s2.y_ });
		float fy1 = std::max({ T.s0.y_, T.s1.y_, T.s2.y_ });

		// Clamp to [0..width-1], [0..height-1]
		int minX = std::max(0, int(std::floor(fx0)));
		int maxX = std::min(w - 1, int(std::ceil(fx1)));
		int minY = std::max(0, int(std::floor(fy0)));
		int maxY = std::min(h - 1, int(std::ceil(fy1)));

		return { minX, maxX, minY, maxY };
	}

	// Edge-function, barycentric weight, triangle contains point
	Bary computeBary(const ScreenTriangle3D& T, const Vec3& p)
	{
		float totalArea = edgeFn(T.s0, T.s1, T.s2);

		float alpha = edgeFn(p, T.s1, T.s2) / totalArea;
		float beta = edgeFn(p, T.s2, T.s0) / totalArea;
		float gamma = edgeFn(p, T.s0, T.s1) / totalArea;

		return { alpha, beta, gamma };
	}
	float edgeFn(const Vec3& a, const Vec3& b, const Vec3& p)
	{
		// AB x AP = (b - a) x (p - a)
		return (b.x_ - a.x_) * (p.y_ - a.y_) - (b.y_ - a.y_) * (p.x_ - a.x_);
	}
	bool contains(const ScreenTriangle3D& T, const Vec3& p)
	{
		Bary bary = computeBary(T, p);
		return (bary.alpha >= 0 && bary.beta >= 0 && bary.gamma >= 0);
	}

	// Depth and color interpolation (persp-correct)
	float interpDepth(const ScreenTriangle3D& T, Bary b)
	{
		return b.alpha * T.s0.z_ + b.beta * T.s1.z_ + b.gamma * T.s2.z_;
	}
	Color interpColor(const ScreenTriangle3D& T, Bary bary)
	{
		float oneOverW = bary.alpha * T.invW[0] + bary.beta * T.invW[1]
			+ bary.gamma * T.invW[2];

		float r = (bary.alpha * T.rOverW[0] + bary.beta * T.rOverW[1]
			+ bary.gamma * T.rOverW[2]) / oneOverW;
		float g = (bary.alpha * T.gOverW[0] + bary.beta * T.gOverW[1]
			+ bary.gamma * T.gOverW[2]) / oneOverW;
		float b = (bary.alpha * T.bOverW[0] + bary.beta * T.bOverW[1]
			+ bary.gamma * T.bOverW[2]) / oneOverW;

		return Color(r, g, b);
	}
}
