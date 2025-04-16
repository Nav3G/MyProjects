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

    // Member variables
    Vec3 v0, v1, v2;
    Color color0, color1, color2;

    // Constructor
    Triangle(const Vec3& a, const Vec3& b, const Vec3& c, 
        const Color col0, const Color col1, const Color col2); // Color attributed to each vertex
    Triangle(const Vec3& a, const Vec3& b, const Vec3& c); // Plain vertex location

    // Utility functions
    // 1) Edgefunction and finding barycentrics
    float edgeFunction(const Vec3& a, const Vec3& b, const Vec3& p) const;
    Barycentrics computeBarycentrics(const Vec3& p) const;

    Color interpolateColor(Barycentrics bary) const;
    bool contains(const Vec3& p) const;
};
