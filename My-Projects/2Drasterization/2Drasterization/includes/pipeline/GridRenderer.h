#pragma once
#include <vector>
#include "core/Vec3.h"
#include "core/Color.h"
#include "scene/Camera.h"
#include "pipeline/RasterUtils.h"
#include "scene/Triangle3D.h"


class GridRenderer
{
private:
	struct Line { Vec3 p0, p1; };
	std::vector<Line> worldLines_;	// stored in world space

	void generateWorldLines(int minX, int maxX, int minZ, int maxZ, float spacing);

public:
	// Constructor
	GridRenderer(int minX, int maxX, int minZ, int maxZ, float spacing);

	// Draw lines
	void draw(const Camera& cam, Framebuffer& fb, float nearPlane) const;
	std::vector<Triangle3D> generateGridQuads(float thickness) const;

	// Helper
	float deg2rad(float d) const { return d * 3.14159265f / 180.0f; }
};

