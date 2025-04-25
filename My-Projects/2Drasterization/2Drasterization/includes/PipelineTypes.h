#pragma once
#include <array>
#include "Vec4.h"
#include "Color.h"
#include <cmath>

namespace Pipeline {
	// After vertex processing (MVP + cull + frustum reject), you have a triangle
	// of 3 clip-space vertices, each carrying whatever varyings needed.
	struct Vertex {
		Vec4   clipPos;    // clip-space (x,y,z,w)
		Color  color;      // per-vertex color, or other varyings (normals, UVs, etc.)

	};

	using Primitive = std::array<Vertex, 3>;  // one triangle

	// Once we rasterize a Primitive into a pixel, we compute a Fragment.
	// It holds exactly the data needed by the fragmentStage to do depth tests,
	// shading, blending, and write to the framebuffer.
	struct Fragment {
		float  depth;      // interpolated depth
		Color  color;      // interpolated/perspective-corrected color
		// ... could add normal, UV, material ID, etc. here
	};
}
