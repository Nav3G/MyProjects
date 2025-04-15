#include "Triangle.h"
#include "Color.h"

#include <fstream>
#include <algorithm>
#include <vector>
#include <iostream>

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

    // Initialize a triangle object
    Triangle triangle({ 100, 50 }, { 400, 150 }, { 250, 400 });

    // Compute the bounding box around the triangle to minimize calculation
    int minX = std::min({ triangle.v0.x, triangle.v1.x, triangle.v2.x });
    int maxX = std::max({ triangle.v0.x, triangle.v1.x, triangle.v2.x });
    int minY = std::min({ triangle.v0.y, triangle.v1.y, triangle.v2.y });
    int maxY = std::max({ triangle.v0.y, triangle.v1.y, triangle.v2.y });

    minX = std::max(minX, 0);
    maxX = std::min(maxX, W - 1);
    minY = std::max(minY, 0);
    maxY = std::min(maxY, H - 1);

    // Raster loop using the triangle's edge function
    Color triColor = { 0, 255, 0 };

    for (int y = minY; y <= maxY; y++)
    {
        for (int x = minX; x <= maxX; x++)
        {
            // Compute the center of the pixel
            Vec2 p(x + 0.5f, y + 0.5f);

            if (triangle.contains(p))
            {
                // Set the pixel in the framebuffer to the triangle color
                framebuffer[y * W + x] = triColor;
            }
        }
    }

    // Dump to PPM
    std::ofstream ofs("rendered.ppm", std::ios::binary);
    ofs << "P6\n" << W << " " << H << "\n255\n";
    for (size_t i = 0; i < framebuffer.size(); ++i) {
        // PPM requires raw RGB bytes
        ofs.put(framebuffer[i].r);
        ofs.put(framebuffer[i].g);
        ofs.put(framebuffer[i].b);
    }
    ofs.close();
    std::cout << "Wrote output.ppm\n";

    return 0;
}
