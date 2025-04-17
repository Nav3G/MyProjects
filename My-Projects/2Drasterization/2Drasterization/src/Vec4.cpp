#include "Vec4.h"

// Constructors
Vec4::Vec4() : x_(0.0f), y_(0.0f), z_(0.0f), w_(1.0f) {}
Vec4::Vec4(float xVal, float yVal, float zVal, float wVal) : x_(xVal), 
	y_(yVal), z_(zVal), w_(1.0f) {}

// Accesors
float Vec4::x() const
{
	return x_;
}
float Vec4::y() const
{
	return y_;
}
float Vec4::z() const
{
	return z_;
}
float Vec4::w() const
{
	return w_;
}

// Mutators
void Vec4::setX(float x)
{
	x_ = x;
}
void Vec4::setY(float y)
{
	y_ = y;
}
void Vec4::setZ(float z)
{
	z_ = z;
}
void Vec4::setW(float w)
{
	w_ = w;
}

// Element-style access
float Vec4::operator[](int idx) const	// read
{
	if (idx < 0 || idx > 3)
	{
		throw std::out_of_range("Vec4 index out of range (0-3)");
	}
	return *(&x_ + idx);
}
float& Vec4::operator[](int idx)		// write
{
	if (idx < 0 || idx > 3)
	{
		throw std::out_of_range("Vec4 index out of range (0-3)");
	}

	return *(&x_ + idx);
}

// Vector arithmetic
Vec4 Vec4::operator+(const Vec4& other) const
{
	return Vec4(x_ + other.x_, y_ + other.y_, z_ + other.z_, w_ + other.w_);
}
Vec4 Vec4::operator-(const Vec4& other) const
{
	return Vec4(x_ - other.x_, y_ - other.y_, z_ - other.z_, w_ - other.w_);
}
Vec4 Vec4::operator*(float scalar) const
{
	return Vec4(scalar * x_, scalar * y_, scalar * z_, scalar * w_);
}
Vec4 Vec4::operator/(float scalar) const
{
	return Vec4(scalar / x_, scalar / y_, scalar / z_, scalar / w_);
}

// Dot product
float Vec4::dot(const Vec4& other) const
{
	return x_ * other.x_ + y_ * other.y_ + z_ * other.z_ + w_ * other.w_;
}

// Multiply by a 4×4 matrix(to be defined once you have Matrix4)
// Vec4 operator*(const Matrix4& m) const;

Vec4 Vec4::perspectiveDivide() const
{
	return Vec4(x_ / w_, y_ / w_, z_ / w_, 1.0f);
}