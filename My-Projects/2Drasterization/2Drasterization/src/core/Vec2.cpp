#include "core/Vec2.h"

#include <cmath>
#include <stdexcept>

// Constructors
Vec2::Vec2() : x(0), y(0) {}
Vec2::Vec2(float xVal, float yVal) : x(xVal), y(yVal) {}

// Accessors
float Vec2::operator[](int idx) const
{
    if (idx < 0 || idx > 1)
    {
        throw std::out_of_range("Vec4 index out of range (0-1)");
    }
    return *(&x + idx);
}
float& Vec2::operator[](int idx)
{
    if (idx < 0 || idx > 1)
    {
        throw std::out_of_range("Vec4 index out of range (0-1)");
    }
    return *(&x + idx);
}

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
float Vec2::dot(const Vec2& other) const
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
