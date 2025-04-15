#pragma once
#include "Vec2.h"

#include <vector>
#include <iostream>

class Triangle
{
    struct Barycentrics
    {
        float alpha, beta, gamma;
    };

public:
    Vec2 v0, v1, v2;

    // Constructor
    Triangle(const Vec2& a, const Vec2& b, const Vec2& c);

    // Utility functions
    float edgeFunction(const Vec2& a, const Vec2& b, const Vec2& p) const;
    Barycentrics computeBarycentrics(const Vec2& p) const;
    bool contains(const Vec2& p) const;
};
