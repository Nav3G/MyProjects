#include "Renderer.h"

// Constructor
Renderer::Renderer(int width, int height) : width_(width), height_(height) {}

void Renderer::render(const Camera& cam, 
	float FovY, float aspect, float near, float far, 
		const std::vector<Mesh>& scene, Framebuffer& fb)
{
	// 0) Clear the framebuffer
	fb.clearColor({ 150, 150, 150 });
	fb.clearDepth(std::numeric_limits<float>::max());

	// Precompute matrices
	Matrix4 V = cam.getViewMatrix();
	Matrix4 P = cam.getProjMatrix(FovY, aspect, near, far);

	// 1) Vertex stage
	std::vector<Primitive> primitives;
	for (auto& mesh : scene)
	{
		auto local = vertexStage(cam, mesh, P, V, near);
		primitives.insert(primitives.end(), local.begin(), local.end());
	}

	// 2) Clip stage
	//auto clipped = clipStage(primitives);

	// 3) Raster stage
	//rasterStage(clipped, fb);
}

std::vector<Primitive> Renderer::vertexStage(const Camera& cam, const Mesh& mesh,
	const Matrix4& P, const Matrix4& V, const float& near)
{
	std::vector<Pipeline::Primitive> out;

	for (auto& t3 : mesh.triangles)
	{ 
		// World -> Camera
		Vec4 cam0 = V * Vec4(t3.v0.x_, t3.v0.y_, t3.v0.z_, 1.0f);
		Vec4 cam1 = V * Vec4(t3.v1.x_, t3.v1.y_, t3.v1.z_, 1.0f);
		Vec4 cam2 = V * Vec4(t3.v2.x_, t3.v2.y_, t3.v2.z_, 1.0f);

		// Back-face culling
        // Compute triangle normal in camera space
		if (cam0.z() > -near && cam1.z() > -near && cam2.z() > -near) continue; // skip
		Vec3 normal = (cam1 - cam0).cross(cam2 - cam0);
		if (normal.z_ <= 0) continue;											// skip

		// Camera -> Clip
		Vec4 c0 = P * cam0;
		Vec4 c1 = P * cam1;
		Vec4 c2 = P * cam2;

		// Pack into a Primitive
		out.push_back({ Vertex{c0, t3.c0}, Vertex{c1, t3.c1}, Vertex{c2, t3.c2} });
	}
	return out;
}

std::vector<Primitive> Renderer::clipStage(const std::vector<Primitive>& inPrims)
{
	std::vector<Primitive> out;

	for (auto& prim : inPrims)
	{
		// Seed a VertexPoly
		VertexPoly polygon = { prim[0], prim[1], prim[2] };

		// Clip against frustum planes
		for (auto f : { planeLeft, planeRight, planeBottom, planeTop, planeNear, planeFar }) {
			polygon = clipPolygon(polygon, f);
			if (polygon.empty()) break;
		}
		if (polygon.empty()) continue;  // fully culled

		// Fan out the newly clipped polygon
		auto fans = triangulateFan(polygon);

		// Push to new primitives vector as a triangle primitve
		for (auto& fan : fans) {
			out.push_back({ fan[0], fan[1], fan[2] });
		}
	}
	return out;
}

void Renderer::rasterStage(const std::vector<Primitive>& inPrims, Framebuffer& fb)
{
	for (auto& prim : inPrims)
	{
		// Perspecive divide: Clip -> NDC i.e. / w = -P_zcam 
		Vec4 ndc0 = prim[0].clipPos.perspectiveDivide();
		Vec4 ndc1 = prim[1].clipPos.perspectiveDivide();
		Vec4 ndc2 = prim[2].clipPos.perspectiveDivide();

		// NDC -> screen space
		Vec3 s0 = fb.toScreen(ndc0);
		Vec3 s1 = fb.toScreen(ndc1);
		Vec3 s2 = fb.toScreen(ndc2);

		// Initialize and compute the bounding box for the given triangle
		BBox bbox = computeBBox(s0, s1, s2, fb.getWidth(), fb.getHeight());

		// Loop over pixels in the bounding box
		for (int y = bbox.minY; y <= bbox.maxY; y++)
		{
			for (int x = bbox.minX; x <= bbox.maxX; x++)
			{
				// Get the center of a pixel
				
			}
		}
	}
}

void Renderer::fragmentStage(int x, int y, const Pipeline::Fragment& f,
	Framebuffer& fb)
{

}

struct BBox { int minX, maxX, minY, maxY; };

static BBox computeBBox(const Vec3& s0,
	const Vec3& s1,
	const Vec3& s2,
	int width, int height)
{
	// compute min/max in float
	float fx0 = std::min({ s0.x_, s1.x_, s2.x_ });
	float fx1 = std::max({ s0.x_, s1.x_, s2.x_ });
	float fy0 = std::min({ s0.y_, s1.y_, s2.y_ });
	float fy1 = std::max({ s0.y_, s1.y_, s2.y_ });

	// clamp to [0..width-1], [0..height-1]
	int minX = std::max(0, int(std::floor(fx0)));
	int maxX = std::min(width - 1, int(std::ceil(fx1)));
	int minY = std::max(0, int(std::floor(fy0)));
	int maxY = std::min(height - 1, int(std::ceil(fy1)));

	return { minX, maxX, minY, maxY };
}