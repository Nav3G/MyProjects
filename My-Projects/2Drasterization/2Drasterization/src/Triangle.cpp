#include "Triangle.h"

Triangle::Triangle(const Vec2& a, const Vec2& b, const Vec2& c)
{
    v0 = a;
    v1 = b;
    v2 = c;
}

float Triangle::edgeFunction(const Vec2& a, const Vec2& b, const Vec2& p) const
{
    // (b - a) x (p - a)
    return (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
}

Triangle::Barycentrics Triangle::computeBarycentrics(const Vec2& p) const
{
    float totalArea = edgeFunction(v0, v1, v2);

    float alpha = edgeFunction(p, v1, v2) / totalArea;
    float beta = edgeFunction(p, v2, v0) / totalArea;
    float gamma = edgeFunction(p, v0, v1) / totalArea;

    return { alpha, beta, gamma };
}

bool Triangle::contains(const Vec2& p) const
{
    Barycentrics bary = computeBarycentrics(p);
    return (bary.alpha >= 0 && bary.beta >= 0 && bary.gamma >= 0);
}

