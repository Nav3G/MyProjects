#pragma once

#include "Color.h"
#include <vector>
#include <limits>
#include <iostream>
#include <string>
#include <fstream>

class Framebuffer {
private:
    int width, height;
    std::vector<Color> colorBuffer;
    std::vector<float> depthBuffer;
public:
    // Constructor: allocate buffers with given dimensions
    Framebuffer(int w, int h);

    // Methods to clear buffers
    void clearColor(Color bg);
    void clearDepth(float initialDepth);

    // Set a pixel: update both color and depth if needed
    void setPixel(int x, int y, Color color, float depth);

    // Method to export to a PPM (or another image format)
    void saveToPPM(const std::string& filename) const;

    // Utility methods, such as getting width and height
    int getHeight();
    int getWidth();
};

