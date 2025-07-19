#include <fstream>
#include <vector>
#include <iostream>

struct Vec2 { float x, y; };
struct Color { uint8_t r, g, b; };

int EdgeLeft(const Vec2& A, const Vec2& B, const Vec2& P)
{
	// AB x AP = (B−A) x (P−A)
	return (B.x - A.x) * (P.y - A.y) - (B.y - A.y) * (P.x - A.x);
}

int main()
{
	// Initialize a flat framebuffer of Color objects
	const int W = 512, H = 512;
	std::vector<Color> framebuffer(W * H);

	// Clear the framebuffer: set every pixel to gray
	Color bg{ 30, 30, 30 };
	for (int y = 0; y < H; ++y)
	{
		for (int x = 0; x < W; ++x)
		{
			size_t idx = y * W + x;
			framebuffer[idx] = bg;
		}
	}

	/// Alternatively : std::fill(framebuffer.begin(), framebuffer.end(), bg);

	// Initialize three random vertices for a triangle
	Vec2 v0 = { 100, 100 };
	Vec2 v1 = { 400, 150 };
	Vec2 v2 = { 250, 400 };

	// Compute the bounding box around the triangle to minimize calculation
	int minX = std::min({ v0.x, v1.x, v2.x });
	int maxX = std::max({ v0.x, v1.x, v2.x });
	int minY = std::min({ v0.y, v1.y, v2.y });
	int maxY = std::max({ v0.y, v1.y, v2.y });

	minX = std::max(minX, 0);
	maxX = std::min(maxX, W - 1);
	minY = std::max(minY, 0);
	maxY = std::min(maxY, H - 1);

	// Raster loop using edge function
	Color triColor = { 0, 255, 0 };

	for (int y = minY; y <= maxY; ++y) 
	{
		for (int x = minX; x <= maxX; ++x) 
		{
			// 1) form the sample point at the pixel center
			Vec2 P{ x + 0.5f, y + 0.5f };

			// 2) run the three edge tests
			if (EdgeLeft(v0, v1, P) >= 0 &&
				EdgeLeft(v1, v2, P) >= 0 &&
				EdgeLeft(v2, v0, P) >= 0)
			{
				// 3) compute the flat index
				size_t idx = y * W + x;
				// 4) write your triangle color
				framebuffer[idx] = triColor;
			}
		}
	}

	// 5) Dump to PPM
	std::ofstream ofs("output.ppm", std::ios::binary);
	ofs << "P6\n" << W << " " << H << "\n255\n";
	for (size_t i = 0; i < framebuffer.size(); ++i) {
		// PPM wants raw RGB bytes
		ofs.put(framebuffer[i].r);
		ofs.put(framebuffer[i].g);
		ofs.put(framebuffer[i].b);
	}
	ofs.close();
	std::cout << "Wrote output.ppm\n";

	return 0;
};