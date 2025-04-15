#include "Vec2.h"

#include <cmath>
#include <stdexcept>

// Constructors
Vec2::Vec2() : x(0), y(0) {}
Vec2::Vec2(float xVal, float yVal) : x(xVal), y(yVal) {}

// Operator overloads
Vec2 Vec2::operator+(const Vec2& other) const
{
    return Vec2(x + other.x, y + other.y);
}
Vec2 Vec2::operator-(const Vec2& other) const
{
    return Vec2(x - other.x, y - other.y);
}
Vec2 Vec2::operator*(float scalar) const
{
    return Vec2(x * scalar, y * scalar);
}
Vec2 Vec2::operator/(float scalar) const
{
    if (scalar == 0)
    {
        throw std::runtime_error("Division by zero error!");
    }
    return Vec2(x / scalar, y / scalar);
}

// Utility Methods
float Vec2::dotProd(const Vec2& other) const
{
    return x * other.x + y * other.y;
}
float Vec2::magnitude() const
{
    return std::sqrt(x * x + y * y);
}
Vec2 Vec2::normalize() const {
    float m = magnitude();
    if (m == 0) {
        throw std::runtime_error("Cannot normalize a zero-length vector.");
        // Alternatively, you could return Vec2(0, 0);
    }
    return *this / m;
}
