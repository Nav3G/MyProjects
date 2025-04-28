#include "pipeline/GridRenderer.h"

using namespace RasterUtils;

// Constructor
GridRenderer::GridRenderer(int minX, int maxX, int minZ, int maxZ, float spacing)
{
	generateWorldLines(minX, maxX, minZ, maxZ, spacing);
}

// Line generation
void GridRenderer::generateWorldLines(int minX, int maxX, int minZ, int maxZ, float spacing)
{
	for (int x = minX; x <= maxX; x += spacing)
	{
		worldLines_.push_back({ Vec3(x, 0, minZ), Vec3(x, 0, maxZ) });
	}
	for (int z = minZ; z <= maxZ; z += spacing)
	{
		worldLines_.push_back({ Vec3(minX, 0, z), Vec3(maxX, 0, z) });
	}
}

// Draw grid
void GridRenderer::draw(const Camera& cam, Framebuffer& fb, float nearPlane) const
{
	auto V = cam.getViewMatrix();
	auto P = Matrix4::perspective(deg2rad(cam.getZoom()),
		float(fb.getWidth()) / fb.getHeight(),
		nearPlane, /*far=*/100.0f);

	for (auto& line : worldLines_)
	{
		// 1) Transform to camera-space
		Vec4 v0 = V * Vec4(line.p0.x_, line.p0.y_, line.p0.z_, 1.0f);
		Vec4 v1 = V * Vec4(line.p1.x_, line.p1.y_, line.p1.z_, 1.0f);
		// std::cout << v0.z() << ", " << v1.z() << "\n";

		// 2) cull before projection:
		//    if both endpoints are _behind_ nearPlane, skip
		if (v0.z() > -nearPlane || v1.z() > -nearPlane) continue;

		// 3) Project to clip space
		Vec4 c0 = P * v0;
		Vec4 c1 = P * v1;
		c0 = c0.perspectiveDivide();
		c1 = c1.perspectiveDivide();
		Vec3 s0 = fb.toScreen(c0);
		Vec3 s1 = fb.toScreen(c1);

		// 4) Screen-bounds reject
		if ((s0.x_ < 0 && s1.x_ < 0) || (s0.x_ > fb.getWidth() && s1.x_ > fb.getWidth()) ||
			(s0.y_ < 0 && s1.y_ < 0) || (s0.y_ > fb.getHeight() && s1.y_ > fb.getHeight()))
			continue;

		// 5) Draw the line (add to framebuffer)
		drawLine(Vec2(s0[0], s0[1]), Vec2(s1[0], s1[1]), s0.z_, s1.z_, { 0, 0, 0 }, fb);
	}
}

std::vector<Triangle3D> GridRenderer::generateGridQuads(float thickness) const
{
	std::vector<Triangle3D> quads;
	quads.reserve(worldLines_.size() * 2);

	float halfT = thickness * 0.5f;
	for (auto& seg : worldLines_)
	{
		Vec3 d = seg.p0 - seg.p1;
		Vec3 dir2(d.x_, 0, d.z_);
		Vec3 n = dir2.cross(Vec3(0.0f, 1.0f, 0.0f)).normalize();

		Vec3 delta = n * halfT;

		// Build quad corners
		Vec3 A = seg.p0 + delta;
		Vec3 B = seg.p0 - delta;
		Vec3 C = seg.p1 - delta;
		Vec3 D = seg.p1 + delta;

		// Build Triangle3Ds
		quads.push_back(Triangle3D(A, B, C, { 0,0,0 }, { 0,0,0 }, { 0,0,0 }));
		quads.push_back(Triangle3D(A, C, D, { 0,0,0 }, { 0,0,0 }, { 0,0,0 }));
	}
	return quads;
}
