#pragma once
#include <vector>
#include "Mesh.h"              // for std::vector<Mesh>, Triangle3D
#include "PipelineTypes.h"     // for Pipeline::Vertex, Primitive, Fragment
#include "Camera.h"            // your existing Camera
#include "Framebuffer.h"       // your existing Framebuffer
#include "GeometryUtils.h"     // for clipPolygon, intersectPlane, triangulateFanVertices
#include "Vec3.h"

using namespace Pipeline;
using namespace GeometryUtils;

class Renderer
{
private:
    // Stage 1: Project world triangles -> clip-space Primitives
    std::vector<Primitive> vertexStage(const Camera& cam, const Mesh& mesh, 
        const Matrix4& P, const Matrix4& V, const float& near);

    // Stage 2: Clip Primitives against all frustum planes
    std::vector<Primitive> clipStage(const std::vector<Primitive>& inPrims);

    // Stage 3: Rasterize each Primitive into Fragments
    void rasterStage(const std::vector<Primitive>& inPrims, Framebuffer& fb);

    // Stage 4: Perform depth test + write each Fragment
    void fragmentStage(int x, int y, const Pipeline::Fragment& f,
        Framebuffer& fb);

    int width_, height_;

public:
    Renderer(int width, int height);

    void render(const Camera& cam,
        float FovY, float aspect, float near, float far,
        const std::vector<Mesh>& scene, Framebuffer& fb);
};

