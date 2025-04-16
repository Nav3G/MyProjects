#include "Vec3.h"


// Constructors: default and parameterized
Vec3::Vec3() : x(0), y(0), z(0) {}
Vec3::Vec3(float xVal, float yVal, float zVal) : x(xVal), y(yVal), z(zVal) {}

// Operator overloads
Vec3 Vec3::operator+(const Vec3& other) const
{
	return Vec3(x + other.x, y + other.y, z + other.z);
}
Vec3 Vec3::operator-(const Vec3& other) const
{
	return Vec3(x - other.x, y - other.y, z - other.z);
}
Vec3 Vec3::operator*(float scalar) const
{
	return Vec3(x * scalar, y * scalar, z * scalar);
}
Vec3 Vec3::operator/(float scalar) const
{
	if (scalar == 0)
	{
		throw std::runtime_error("Division by zero error!");
	}
	return Vec3(x / scalar, y / scalar, z / scalar);
}

// Utility functions
float Vec3::dot(const Vec3& other) const
{
	return x * other.x + y * other.y + z * other.z;
}
Vec3 Vec3::cross(const Vec3& other) const
{
	return Vec3(y * other.z - z * other.y, z * other.x - x * other.z, 
		x * other.y - y * other.x);
}
float Vec3::magnitude() const
{
	return std::sqrt(x * x + y * y + z * z);
}
Vec3 Vec3::normalize() const
{
	float m = magnitude();
	if (m == 0) {
		throw std::runtime_error("Cannot normalize a zero-length vector.");
		// Alternatively, you could return Vec3(0, 0);
	}
	return *this / m;
}