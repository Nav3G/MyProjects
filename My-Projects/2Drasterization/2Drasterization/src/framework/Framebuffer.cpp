#include "framework/Framebuffer.h"

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
void Framebuffer::drawLine(const Vec2& p0, const Vec2& p1, float z0, float z1, Color c)
{
    // Convert to integer pixel coords
    int x0 = int(std::floor(p0.x));
    int y0 = int(std::floor(p0.y));
    int x1 = int(std::floor(p1.x));
    int y1 = int(std::floor(p1.y));

    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = (x0 < x1 ? 1 : -1);
    int sy = (y0 < y1 ? 1 : -1);
    int err = dx - dy;

    int x = x0, y = y0;
    while (true) {
        // 1) Compute interpolation factor t without risking 0/0
        float t = 0.0f;
        if (dx >= dy) {
            int span = x1 - x0;
            if (span != 0) t = float(x - x0) / float(span);
        }
        else {
            int span = y1 - y0;
            if (span != 0) t = float(y - y0) / float(span);
        }

        // 2) Interpolate depth
        float depth = (1.0f - t) * z0 + t * z1;

        // 3) *Only* read/write the depthBuffer if x,y are in [0..width-1]×[0..height-1]
        if (x >= 0 && x < width && y >= 0 && y < height) {
            size_t idx = size_t(y) * width + size_t(x);
            if (depth < depthBuffer[idx]) {
                // safe to write both colorBuffer and depthBuffer
                setPixel(x, y, c, depth);
            }
        }

        // Advance Bresenham
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
