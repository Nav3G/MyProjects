#include "Matrix4.h"

// Constructors
Matrix4::Matrix4() 
{
	for (int r = 0; r < 4; ++r)
	{
		for (int c = 0; c < 4; ++c)
		{
			m_[r * 4 + c] = 0.0f;
		}
	}
}
Matrix4 Matrix4::identity() 
{
	Matrix4 I;
	for (int i = 0; i < 4; ++i)
	{
		I.m_[i * 4 + i] = 1.0f;
	}
	return I;
}

// Element access
float Matrix4::operator()(int row, int col) const	// read
{
	if (row < 0 || row >= 4 || col < 0 || col >= 4)
	{
		throw std::out_of_range("matrix entry out of range (0-4, 0-4)");
	}
	return m_[row * 4 + col];
}
float& Matrix4::operator()(int row, int col)		// write
{
	if (row < 0 || row >= 4 || col < 0 || col >= 4)
	{
		throw std::out_of_range("matrix entry out of range (0-4, 0-4)");
	}
	return m_[row * 4 + col];
}

// Matrix multiplication
Matrix4 Matrix4::operator*(const Matrix4& RHS) const
{
	Matrix4 result;
	
	for (int r = 0; r < 4; ++r)
	{
		for (int c = 0; c < 4; ++c)
		{
			float sum = 0.0f;
			for (int k = 0; k < 4; ++k)
			{
				sum += m_[r * 4 + k] * RHS.m_[k * 4 + c];
			}
			result.m_[r * 4 + c] = sum;
		}
	}
	return result;
}

// Vector multiplication
Vec4 Matrix4::operator*(const Vec4& v) const
{
	Vec4 result;

	for (int r = 0; r < 4; ++r)
	{
		float sum = 0.0f;
		for (int c = 0; c < 4; ++c)
		{
			sum += m_[r * 4 + c] * v[c];
		}
		result[r] = sum;
	}
	return result;
}

// Transformations
/// Translation
Matrix4 Matrix4::translation(float tx, float ty, float tz)
{
	Matrix4 T = Matrix4::identity();
	T.m_[3] = tx;
	T.m_[7] = ty;
	T.m_[11] = tz;

	return T;
}

/// Scaling
Matrix4 Matrix4::scaling(float sx, float sy, float sz)
{
	Matrix4 S;
	S.m_[0] = sx;
	S.m_[5] = sy;
	S.m_[10] = sz;
	S.m_[15] = 1.0f;

	return S;
}

/// RotationX
Matrix4 Matrix4::rotationX(float radians)
{
	Matrix4 R;
	R.m_[0] = 1.0f;
	R.m_[5] = std::cos(radians);
	R.m_[6] = -std::sin(radians);
	R.m_[9] = std::sin(radians);
	R.m_[10] = std::cos(radians);
	R.m_[15] = 1.0f;

	return R;
}
/// RotationY
Matrix4 Matrix4::rotationY(float radians)
{
	Matrix4 R;
	R.m_[0] = std::cos(radians);
	R.m_[2] = std::sin(radians);
	R.m_[6] = 1.0f;
	R.m_[8] = -std::sin(radians);
	R.m_[10] = std::cos(radians);
	R.m_[15] = 1.0f;

	return R;
}
/// RotationZ
Matrix4 Matrix4::rotationZ(float radians)
{
	Matrix4 R;
	R.m_[0] = std::cos(radians);
	R.m_[1] = -std::sin(radians);
	R.m_[4] = std::sin(radians);
	R.m_[5] = std::cos(radians);
	R.m_[10] = 1.0f;
	R.m_[15] = 1.0f;

	return R;
}

// Camera
/// LookAt
// We form the camera basis, {facing, right, trueUp}, and then rotate it into the 
// world space, aligning its normal with the -Z axis. Then, we just want to center 
// the camera in world space, so that means simply translate (subtract)
// it to the center now that its all rotated nicely.
Matrix4 Matrix4::lookAt(const Vec3& eye, const Vec3& target, const Vec3& up)
{
	// 1) build camera axes
	Vec3 f = (target - eye).normalize();      // forward
	Vec3 r = f.cross(up).normalize();         // right
	Vec3 u = r.cross(f);                      // true up

	// 2) start with identity
	Matrix4 M = Matrix4::identity();

	// 3) fill the rotation (upper-left 3x3)
	//    [ r.x  u.x  -f.x ]
	//    [ r.y  u.y  -f.y ]
	//    [ r.z  u.z  -f.z ]
	M(0, 0) = r.x_;  M(0, 1) = u.x_;  M(0, 2) = -f.x_;
	M(1, 0) = r.y_;  M(1, 1) = u.y_;  M(1, 2) = -f.y_;
	M(2, 0) = r.z_;  M(2, 1) = u.z_;  M(2, 2) = -f.z_;

	// 4) fill the translation (last column)
	M(0, 3) = -r.dot(eye);
	M(1, 3) = -u.dot(eye);
	M(2, 3) = f.dot(eye);
	// bottom-right stays 1
	M(3, 3) = 1.0f;

	return M; 
}
/// Orthographic
Matrix4 Matrix4::orthographic(float left, float right,
	float bottom, float top,
	float near, float far)
{
	// Start with a zero matrix
	Matrix4 O;

	// Scale X, Y, Z to the [-1,1] range
	O.m_[0 * 4 + 0] = 2.0f / (right - left);    // row0, col0
	O.m_[1 * 4 + 1] = 2.0f / (top - bottom);  // row1, col1
	O.m_[2 * 4 + 2] = -2.0f / (far - near);    // row2, col2

	// Translate centers to origin
	O.m_[0 * 4 + 3] = -(right + left) / (right - left);  // row0, col3
	O.m_[1 * 4 + 3] = -(top + bottom) / (top - bottom);// row1, col3
	O.m_[2 * 4 + 3] = -(far + near) / (far - near);  // row2, col3

	// Bottom-right corner stays 1
	O.m_[3 * 4 + 3] = 1.0f;               // row3, col3

	return O;
}
/// Perspective
// Transforms each world space coordinate to the near clipping plane
Matrix4 Matrix4::perspective(float fovY, float aspect,
	float near, float far)
{
	// Start with a zero matrix
	Matrix4 P;

	// Compute focal scale from vertical FOV
	float f = 1.0f / std::tan(fovY * 0.5f);

	// X and Y scale
	P.m_[0 * 4 + 0] = f / aspect;         // row0, col0
	P.m_[1 * 4 + 1] = f;                  // row1, col1

	// Z remap: [near,far] -> [-1,1]
	P.m_[2 * 4 + 2] = -(far + near) / (far - near);        // row2, col2
	P.m_[2 * 4 + 3] = -2.0f * far * near / (far - near);   // row2, col3

	// W component to perform perspective divide
	P.m_[3 * 4 + 2] = -1.0f;              // row3, col2

	return P;
}