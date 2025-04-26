#pragma once

#include "core/Color.h"

class Vec2
{
public:
    float x, y;

    // Constructors
    Vec2(); // Default
    Vec2(float xVal, float yVal);

    // Accessors
    float operator[](int idx) const;
    float& operator[](int idx);

    // Operator overloads
    Vec2 operator+(const Vec2& other) const;
    Vec2 operator-(const Vec2& other) const;
    Vec2 operator*(float scalar) const;
    Vec2 operator/(float scalar) const;

    // Utility Methods
    float dot(const Vec2& other) const;
    float magnitude() const;
    Vec2 normalize() const;
};
