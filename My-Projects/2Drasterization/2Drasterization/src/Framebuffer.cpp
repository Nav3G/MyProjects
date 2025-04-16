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