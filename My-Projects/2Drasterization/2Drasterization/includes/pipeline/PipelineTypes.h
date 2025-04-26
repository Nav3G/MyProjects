#pragma once
#include "core/Vec4.h"
#include "core/Color.h"
#include <array>
#include <cmath>

namespace Pipeline {
	// After vertex processing (MVP + cull + frustum reject), you have a triangle
	// of 3 clip-space vertices, each carrying whatever varyings needed.
	struct Vertex {
		Vec4   clipPos;    // clip-space (x,y,z,w)
		Color  color;      // per-vertex color, or other varyings (normals, UVs, etc.)

	};

	using Primitive = std::array<Vertex, 3>;  // one triangle

	struct ScreenTriangle3D {
		Vec3 s0, s1, s2;						// screen-space positions after toScreen()
		float invW[3];							// 1/w from clip space
		float rOverW[3], gOverW[3], bOverW[3];  // pre-multiplied colors
	};

	// Once we rasterize a Primitive into a pixel, we compute a Fragment.
	// It holds exactly the data needed by the fragmentStage to do depth tests,
	// shading, blending, and write to the framebuffer.
	struct Fragment {
		float  depth;      // interpolated depth
		Color  color;      // interpolated/perspective-corrected color
		// ... could add normal, UV, material ID, etc. here
	};
}
