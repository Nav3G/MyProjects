#pragma once

#include "pipeline/PipelineTypes.h"

namespace RasterUtils
{
	struct BBox { int minX, maxX, minY, maxY; };	// Bounding box type
	struct Bary { float alpha, beta, gamma; };		// Barycentrics type

	BBox   computeBBox(const Pipeline::ScreenTriangle3D& T, int w, int h);		// Compute the bounding box dimentions
	Bary   computeBary(const Pipeline::ScreenTriangle3D& T, const Vec3& p);		// Compute the barycentric weights
	float  edgeFn(const Vec3& a, const Vec3& b, const Vec3& p);
	bool   contains(const Pipeline::ScreenTriangle3D& T, const Vec3& p);		// Determine if point is inside trianlge boundary
	float  interpDepth(const Pipeline::ScreenTriangle3D& T, Bary b);			// Interpolate depth
	Color  interpColor(const Pipeline::ScreenTriangle3D& T, Bary bary);			// Interpolate color
}