#pragma once

#include <stdexcept>

class Vec4 {
private:
    float x_, y_, z_, w_;   // homogeneous coordinates

public:
    // Constructors
    Vec4();     // default: (0,0,0,1)
    Vec4(float xVal, float yVal, float zVal, float wVal);

    // Accessors
    float x() const;
    float y() const;
    float z() const;
    float w() const;

    // Mutators 
    void setX(float x);
    void setY(float y);
    void setZ(float z);
    void setW(float w);

    // Element-style access
    float operator[](int idx) const;   // read
    float& operator[](int idx);        // write

    // Vector arithmetic
    Vec4 operator+(const Vec4& other) const;
    Vec4 operator-(const Vec4& oother) const;
    Vec4 operator*(float scalar)   const;
    Vec4 operator/(float scalar)   const;

    // Dot product (4D)
    float dot(const Vec4& other) const;

    // Multiply by a 4×4 matrix (to be defined once you have Matrix4)
    // Vec4 operator*(const Matrix4& m) const;

    // Convenience: convert to 3D by performing the perspective divide
    // (i.e. x/=w; y/=w; z/=w; w=1)
    Vec4 perspectiveDivide() const;
};

