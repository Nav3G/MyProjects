#include "Vec3.h"

// Constructors: default and parameterized
Vec3::Vec3() : x_(0), y_(0), z_(0) {}
Vec3::Vec3(float xVal, float yVal, float zVal) : x_(xVal), y_(yVal), z_(zVal) {}

// Operator overloads
Vec3 Vec3::operator+(const Vec3& other) const
{
	return Vec3(x_ + other.x_, y_ + other.y_, z_ + other.z_);
}
Vec3 Vec3::operator-(const Vec3& other) const
{
	return Vec3(x_ - other.x_, y_ - other.y_, z_ - other.z_);
}
Vec3 Vec3::operator*(float scalar) const
{
	return Vec3(x_ * scalar, y_ * scalar, z_ * scalar);
}
Vec3 Vec3::operator/(float scalar) const
{
	if (scalar == 0)
	{
		throw std::runtime_error("Division by zero error!");
	}
	return Vec3(x_ / scalar, y_ / scalar, z_ / scalar);
}

// Utility functions
float Vec3::dot(const Vec3& other) const
{
	return x_ * other.x_ + y_ * other.y_ + z_ * other.z_;
}
Vec3 Vec3::cross(const Vec3& other) const
{
	return Vec3(y_ * other.z_ - z_ * other.y_, z_ * other.x_ - x_ * other.z_,
		x_ * other.y_ - y_ * other.x_);
}
float Vec3::magnitude() const
{
	return std::sqrt(x_ * x_ + y_ * y_ + z_ * z_);
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