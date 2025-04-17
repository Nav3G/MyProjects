#pragma once

#include "Color.h"
#include <cmath>
#include <stdexcept>

class Vec3
{
public:
    float x_, y_, z_;

    // Constructors: default and parameterized
    Vec3(); // Defaults to (0, 0, 0)
    Vec3(float xVal, float yVal, float zVal);

    // Operator overloads: +, -, scalar multiplication and division, etc.
    Vec3 operator+(const Vec3& other) const;
    Vec3 operator-(const Vec3& other) const;
    Vec3 operator*(float scalar) const;
    Vec3 operator/(float scalar) const;

    // Utility methods: dot product, cross product, magnitude, normalization
    float dot(const Vec3& other) const;
    Vec3 cross(const Vec3& other) const;
    float magnitude() const;
    Vec3 normalize() const;
};

