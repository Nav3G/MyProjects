#pragma once
#include <vector>
#include "Vec3.h"
#include "Color.h"
#include "Camera.h"
#include "Framebuffer.h"


class Grid
{
private:
	struct Line { Vec3 p0, p1; };
	std::vector<Line> worldLines_;	// stored in world space

	void generateWorldLines(int minX, int maxX, int minZ, int maxZ, float spacing);

public:
	// Constructor
	Grid(int minX, int maxX, int minZ, int maxZ, float spacing);

	// Draw lines
	void draw(const Camera& cam, Framebuffer& fb, float nearPlane) const;

	// Helper
	float deg2rad(float d) const { return d * 3.14159265f / 180.0f; }
};

