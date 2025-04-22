#include "Framebuffer.h"

// Constructor
Framebuffer::Framebuffer(int w, int h) : width(w), height(h), colorBuffer(w * h, Color()),
	depthBuffer(w * h, std::numeric_limits<float>::max()) {}

// Methods to clear buffers
void Framebuffer::clearColor(Color bg)
{
	std::fill(colorBuffer.begin(), colorBuffer.end(), bg);
}

void Framebuffer::clearDepth(float initialDepth)
{
	std::fill(depthBuffer.begin(), depthBuffer.end(), initialDepth);
}

// Set a pixel: update both color and depth if needed
void Framebuffer::setPixel(int x, int y, Color color, float depth)
{
	colorBuffer[y * width + x] = color;
	depthBuffer[y * width + x] = depth;
}

// Screen space transform
Vec3 Framebuffer::toScreen(const Vec4& ndc)
{
	float x = (ndc.x() * 0.5f + 0.5f) * width;
	float y = (1.0f - (ndc.y() * 0.5f + 0.5f)) * height;
	float z = ndc.z();
	return Vec3(x, y, z);
}

// Utility methods
int Framebuffer::getHeight() {
	return height;
}
int Framebuffer::getWidth() {
	return width;
}
std::vector<Color>& Framebuffer::getColorBuffer()
{
	return colorBuffer;
}
std::vector<float>& Framebuffer::getDepthBuffer()
{
	return depthBuffer;
}
void Framebuffer::setColorBuffer(Color color, int y, int x)
{
	colorBuffer[y * width + x] = color;
}
void Framebuffer::setDepthBuffer(float depth, int y, int x)
{
	depthBuffer[y * width + x] = depth;
}

// Grid drawing
void Framebuffer::drawLine(const Vec2& p0, const Vec2& p1, Color c)
{
	int x0 = int(p0.x), y0 = int(p0.y);
	int x1 = int(p1.x), y1 = int(p1.y);
	int x = x0, y = y0;
	int dx = abs(x1 - x0), dy = abs(y1 - y0);
	int sx = x0 < x1 ? 1 : -1, sy = y0 < y1 ? 1 : -1;
	int err = dx - dy;
	while (true) {
		if (x >= 0 && x < width && y >= 0 && y < height) {
			setPixel(x, y, c, /*depth=*/0);
		}
		if (x == x1 && y == y1) break;
		int e2 = err * 2;
		if (e2 > -dy) { err -= dy; x += sx; }
		if (e2 < dx) { err += dx; y += sy; }
	}
}

// Export to PPM
void Framebuffer::saveToPPM(const std::string& filename) const
{
	std::ofstream ofs("rendered.ppm", std::ios::binary);
	ofs << "P6\n" << width << " " << height << "\n255\n";
	for (size_t i = 0; i < width * height; ++i) {
		// PPM requires raw RGB bytes
		ofs.put(colorBuffer[i].r);
		ofs.put(colorBuffer[i].g);
		ofs.put(colorBuffer[i].b);
	}
	ofs.close();
	std::cout << "Wrote output.ppm\n";
}
