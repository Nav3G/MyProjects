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
	if ((row < 0 || row > 4) && (col < 0 || col > 4))
	{
		throw std::out_of_range("matrix entry out of range (0-4, 0-4)");
	}
	return m_[row, col];
}
float& Matrix4::operator()(int row, int col)		// write
{
	if ((row < 0 || row > 4) && (col < 0 || col > 4))
	{
		throw std::out_of_range("matrix entry out of range (0-4, 0-4)");
	}
	return m_[row, col];
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