#include "Triangle.h"

// Constructors
Triangle::Triangle(const Vec2& a, const Vec2& b, const Vec2& c,
    const Color col0, const Color col1, const Color col2)
{
    v0 = a;
    v1 = b;
    v2 = c;

    color0 = col0;
    color1 = col1;
    color2 = col2;
}

Triangle::Triangle(const Vec2& a, const Vec2& b, const Vec2& c)
{
    v0 = a;
    v1 = b;
    v2 = c;
}

// Utility functions
float Triangle::edgeFunction(const Vec2& a, const Vec2& b, const Vec2& p) const
{
    // AB x AP = (b - a) x (p - a)
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

Color Triangle::interpolateColor(Barycentrics bary) const
{
    float r = bary.alpha * (color0.r) + bary.beta * (color1.r) + bary.gamma * (color2.r);
    float g = bary.alpha * (color0.g) + bary.beta * (color1.g) + bary.gamma * (color2.g);
    float b = bary.alpha * (color0.b) + bary.beta * (color1.b) + bary.gamma * (color2.b);

    return Color(r, g, b);
}