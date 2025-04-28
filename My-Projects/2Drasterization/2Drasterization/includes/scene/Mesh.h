#pragma once
#include <vector>
#include "scene/Triangle3D.h"

class Mesh {
public:
    // Triangles
	std::vector<Triangle3D> triangles;

    Mesh() = default;
    Mesh(std::vector<Triangle3D> tris) : triangles(std::move(tris)) {}
};

