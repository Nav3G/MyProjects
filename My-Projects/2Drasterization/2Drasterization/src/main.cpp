#include "Triangle.h"
#include "Framebuffer.h"
#include "Color.h"

#include <fstream>
#include <vector>
#include <iostream>

int main()
{
    // Initialize a flat framebuffer of Color objects
    Framebuffer fb(500, 500);

    // Initialize a scene: vector of triangles
    Triangle t1({ 100, 50, 0 }, { 400, 150, 0 }, { 250, 400, 0 },
        Color(255, 0, 0), Color(0, 100, 0), Color(0, 0, 255));
    Triangle t2({ 100, 200, 10 }, { 300, 180, 10 }, { 300, 200, 10 },
        Color(255, 0, 0), Color(0, 255, 0), Color(0, 0, 100));
    Triangle t3({ 150, 50, 20 }, { 280, 150, 20 }, { 150, 300, 20 },
        Color(100, 0, 0), Color(0, 255, 0), Color(0, 0, 255));

    std::vector<Triangle> scene = { t1, t2, t3 };

    // Clear each frame
    fb.clearColor(Color(30, 30, 30));
    fb.clearDepth(std::numeric_limits<float>::max());


    for (const Triangle& tri : scene)
    {
        // Compute the bounding box around the triangle to minimize calculation
        int minX = std::min({ tri.v0.x, tri.v1.x, tri.v2.x });
        int maxX = std::max({ tri.v0.x, tri.v1.x, tri.v2.x });
        int minY = std::min({ tri.v0.y, tri.v1.y, tri.v2.y });
        int maxY = std::max({ tri.v0.y, tri.v1.y, tri.v2.y });

        minX = std::max(minX, 0);
        maxX = std::min(maxX, fb.getWidth() - 1);
        minY = std::max(minY, 0);
        maxY = std::min(maxY, fb.getHeight() - 1);

        // Raster loop using the triangle's edge function
        for (int y = minY; y <= maxY; y++)
        {
            for (int x = minX; x <= maxX; x++)
            {
                // 1) Compute the center of the test pixel
                Vec3 p(x + 0.5f, y + 0.5f, 0);

                if (tri.contains(p))
                {
                    // 2) Compute barycentrics for color interpolation
                    Triangle::Barycentrics bary = tri.computeBarycentrics(p);

                    // 3) Interpolate depth (blend them together via the barycentric 
                    // weighting on each vertex
                    float interpDepth = bary.alpha * tri.v0.z +
                        bary.beta * tri.v1.z +
                        bary.gamma * tri.v2.z;

                    // 4) Set the pixel in the framebuffer to the triangle's interpolated color
                    int index = y * fb.getWidth() + x;
                    if (interpDepth < fb.getDepthBuffer()[index]) {  // fb.depthBuffer is your depth buffer array
                        // Update the depth buffer:
                        if (interpDepth < fb.getDepthBuffer()[index]) {  // fb.depthBuffer is your depth buffer array
                            fb.getDepthBuffer()[index] = interpDepth;

                            // Interpolate the color using barycentrics:
                            Color interpColor = tri.interpolateColor(bary);

                            // Set the pixel in the color buffer:
                            fb.setPixel(x, y, interpColor, interpDepth);
                        }
                    }
                }
            }
        }
    }

    // Dump to PPM
    fb.saveToPPM("rendered.ppm");

    return 0;
}