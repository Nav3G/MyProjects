#pragma once
#include "Vec4.h"
#include "Vec3.h"
#include <stdexcept>

class Matrix4
{
private:
	float m_[16];
public:
	// Constructors
	Matrix4();
	static Matrix4 identity();

	// Element access (row, col)
	float operator()(int row, int col) const;
	float& operator()(int row, int col);

	// Matrix multiplication
	Matrix4 operator*(const Matrix4& RHS) const;

	// Vector multiplication
	Vec4 operator*(const Vec4& v) const;

	// Helpers
	static Matrix4 translation(float tx, float ty, float tz);
	static Matrix4 scaling(float sx, float sy, float sz);
	static Matrix4 rotationX(float radians);
	static Matrix4 rotationY(float radians);
	static Matrix4 rotationZ(float radians);
	static Matrix4 perspective(float fovY, float aspect, float near, float far);
	static Matrix4 orthographic(float left, float right, float bottom, float top, float near, float far);
	static Matrix4 lookAt(const Vec3& eye, const Vec3& target, const Vec3& up);
};

