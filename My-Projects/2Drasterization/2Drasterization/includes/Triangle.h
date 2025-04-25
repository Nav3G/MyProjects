#pragma once
#include "Vec2.h"
#include "Vec3.h"
#include "Color.h"

#include <vector>
#include <iostream>

class Triangle
{
public:
    // Struct for type barycenrtics
    struct Barycentrics
    {
        float alpha, beta, gamma;
    };
    // Struct for bouding box dimensions
    struct Boundingbox
    {
        int minX, minY, maxX, maxY;
    };

    // Member variables
    Vec3 v0, v1, v2;
    Color color0, color1, color2;
    float invW[3];
    float rOverW[3], gOverW[3], bOverW[3];

    // Constructor
    Triangle(const Vec3& a, const Vec3& b, const Vec3& c, 
        const Color col0, const Color col1, const Color col2);  // Color attributed to each vertex
    Triangle(const Vec3& a, const Vec3& b, const Vec3& c);      // Plain vertex location

    // Clip space ws
    void preparePerspective(const float clipW[3]);

    // Perpspective correct interp
    Color interpolateColorPC(const Barycentrics& bary) const;

    // Utility functions
    // 1) Edgefunction and finding barycentrics
    float edgeFunction(const Vec3& a, const Vec3& b, const Vec3& p) const;
    Barycentrics computeBarycentrics(const Vec3& p) const;

    // 2) Color/depth interpolation and internal point existence
    Color interpolateColor(Barycentrics bary) const;
    float interpolateDepth(Barycentrics bary) const;
    bool contains(const Vec3& p) const;

    // 3) Bounding box determination 
    Boundingbox getBoundingBox(int fbWidth, int fbHeight) const;

    // Helpers
    Color& colorAt(int i);
    const Color& colorAt(int i) const;
};
