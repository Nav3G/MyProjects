#include "Triangle.h"

// Constructors
Triangle::Triangle(const Vec3& a, const Vec3& b, const Vec3& c,
    const Color col0, const Color col1, const Color col2)
{
    v0 = a;
    v1 = b;
    v2 = c;

    color0 = col0;
    color1 = col1;
    color2 = col2;

    for (int i = 0; i < 3; ++i) {
        invW[i] = 1.0f;                     // “1/w == 1” is a no-op fallback
        rOverW[i] = colorAt(i).r;            // fallback to plain color.r
        gOverW[i] = colorAt(i).g;
        bOverW[i] = colorAt(i).b;
    }
}

Triangle::Triangle(const Vec3& a, const Vec3& b, const Vec3& c)
{
    v0 = a;
    v1 = b;
    v2 = c;

    for (int i = 0; i < 3; ++i) {
        invW[i] = 1.0f;                     // “1/w == 1” is a no-op fallback
        rOverW[i] = colorAt(i).r;            // fallback to plain color.r
        gOverW[i] = colorAt(i).g;
        bOverW[i] = colorAt(i).b;
    }
}

// Utility functions
float Triangle::edgeFunction(const Vec3& a, const Vec3& b, const Vec3& p) const
{
    // AB x AP = (b - a) x (p - a)
    return (b.x_ - a.x_) * (p.y_ - a.y_) - (b.y_ - a.y_) * (p.x_ - a.x_);
}

// Barycentric coordinates: The cross product between the vector spanning a vertex v and
// a point p (v -> p) and the vector representing an edge (rooted at the same vertex v) will result in
// the signed area of the sub-triangle spanned by those two vectors. If the area is small, then
// p must be near the given edge (the two vectors are almost aligned). This area, as a fraction
// of the total area of the main triangle, is then assigned to the vertex opposite the given
// edge. That is, said vertex is given a weighting depending on its distance from p 
// (nearer --> more weight). Note: the centroid is where all weights are equal, i.e. the COM.
// Also note: the sum of the weghts alpha, beta, gamma is 1.
Triangle::Barycentrics Triangle::computeBarycentrics(const Vec3& p) const
{
    float totalArea = edgeFunction(v0, v1, v2);

    float alpha = edgeFunction(p, v1, v2) / totalArea;
    float beta = edgeFunction(p, v2, v0) / totalArea;
    float gamma = edgeFunction(p, v0, v1) / totalArea;

    return { alpha, beta, gamma };
}

// If the barycentric weightings of a point relative to its triangle's vertices are all positive,
// that point must lie inside the triangle, assuming a CW winding.
bool Triangle::contains(const Vec3& p) const
{
    Barycentrics bary = computeBarycentrics(p);
    return (bary.alpha >= 0 && bary.beta >= 0 && bary.gamma >= 0);
}

// We can use the barycentric coordinate of a point to assign it an attribute. In this case, color. 
// Then, the color of a point inside the triangle will represent its barycentric weight. This gives 
// the effect of a gradient. Notice we take an area-normalized average of the color at that point.
Color Triangle::interpolateColor(Barycentrics bary) const
{
    float r = bary.alpha * (color0.r) + bary.beta * (color1.r) + bary.gamma * (color2.r);
    float g = bary.alpha * (color0.g) + bary.beta * (color1.g) + bary.gamma * (color2.g);
    float b = bary.alpha * (color0.b) + bary.beta * (color1.b) + bary.gamma * (color2.b);

    return Color(r, g, b);
}

// The same is true for depth. We can assign a depth to each point relatative to its barycentric
// weighting (coordinate). Since depth is a phsyical coordinate too, this is literally telling us
// the depth of each point in the triangle, assuming the triangle is just a flat plane (always true).
float Triangle::interpolateDepth(Barycentrics bary) const
{
    return bary.alpha * v0.z_ + bary.beta * v1.z_ + bary.gamma * v2.z_;
}

Triangle::Boundingbox Triangle::getBoundingBox(int fbWidth, int fbHeight) const
{
    Boundingbox bbox;
    bbox.minX = std::max(0, static_cast<int>(std::floor(std::min({ v0.x_, v1.x_, v2.x_ }))));
    bbox.maxX = std::min(fbWidth - 1, static_cast<int>(std::ceil(std::max({ v0.x_, v1.x_, v2.x_ }))));
    bbox.minY = std::max(0, static_cast<int>(std::floor(std::min({ v0.y_, v1.y_, v2.y_ }))));
    bbox.maxY = std::min(fbHeight - 1, static_cast<int>(std::ceil(std::max({ v0.y_, v1.y_, v2.y_ }))));
    return bbox;
}

// Perspective correct interp
void Triangle::preparePerspective(const float clipW[3])
{
    for (int i = 0; i < 3; ++i) {
        invW[i] = 1.0f / clipW[i];
        rOverW[i] = colorAt(i).r * invW[i];
        gOverW[i] = colorAt(i).g * invW[i];
        bOverW[i] = colorAt(i).b * invW[i];
    }
}
Color Triangle::interpolateColorPC(const Barycentrics& bary) const
{
    float oneOverW = bary.alpha * invW[0]
        + bary.beta * invW[1]
        + bary.gamma * invW[2];

    float r = (bary.alpha * rOverW[0]
        + bary.beta * rOverW[1]
        + bary.gamma * rOverW[2]) / oneOverW;
    float g = (bary.alpha * gOverW[0]
        + bary.beta * gOverW[1]
        + bary.gamma * gOverW[2]) / oneOverW;
    float b = (bary.alpha * bOverW[0]
        + bary.beta * bOverW[1]
        + bary.gamma * bOverW[2]) / oneOverW;

    return Color(r, g, b);
}

// Helpers
Color& Triangle::colorAt(int i) {
    return (i == 0 ? color0 : (i == 1 ? color1 : color2));
}
const Color& Triangle::colorAt(int i) const
{
    return (i == 0 ? color0 : (i == 1 ? color1 : color2));
}