#pragma once
#include "core/Vec3.h"
#include "core/Color.h"

struct Triangle3D {
	Vec3  v0, v1, v2;
	Color c0, c1, c2;

    Triangle3D(const Vec3& a, const Vec3& b, const Vec3& c,
        const Color& col0, const Color& col1, const Color& col2)
        : v0(a), v1(b), v2(c), c0(col0), c1(col1), c2(col2) {}
};