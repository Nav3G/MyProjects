#pragma once

#include "Color.h"

class Vec2
{
public:
    float x, y;

    // Constructors
    Vec2(); // Default
    Vec2(float xVal, float yVal);

    // Operator overloads
    Vec2 operator+(const Vec2& other) const;
    Vec2 operator-(const Vec2& other) const;
    Vec2 operator*(float scalar) const;
    Vec2 operator/(float scalar) const;

    // Utility Methods
    float dotProd(const Vec2& other) const;
    float magnitude() const;
    Vec2 normalize() const;
};
