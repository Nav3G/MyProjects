#pragma once
#include "core/Vec4.h"
#include "core/Color.h"
#include <array>
#include <cmath>

namespace Pipeline 
{
	// After vertex processing (MVP + cull + frustum reject), we have a triangle
	// of 3 clip-space vertices, each carrying whatever varyings needed.
	struct Vertex {
		Vec4 clipPos;    // clip-space (x,y,z,w)
		Color color;     // per-vertex color, or other varyings (normals, UVs, etc.)
	};

	// This is the triangle in clip-space.
	struct Primitive {
		Vertex v0, v1, v2;

		// Allow indexed access exactly as before:
		Vertex& operator[](int i) { return (i == 0 ? v0 : (i == 1 ? v1 : v2)); }
		const Vertex& operator[](int i) const { return (i == 0 ? v0 : (i == 1 ? v1 : v2)); }

		// Array length helpers
		Vertex* begin() { return &v0; }
		Vertex* end() { return &v0 + 3; }
		const Vertex* begin() const { return &v0; }
		const Vertex* end()   const { return &v0 + 3; }
	};	

	// Since we need to keep clip-space data in tact before we convert to screen-space,
	// and we also don't work with homogenous coordinates, we swap back to 3D screen-space
	// triangle that keeps track of all the previous clip-space vertex information.
	struct ScreenTriangle3D {
		Vec3 s0, s1, s2;						// screen-space positions after toScreen()
		float invW[3];							// 1/w from clip space
		float rOverW[3], gOverW[3], bOverW[3];  // pre-multiplied colors
	};

	// Once we rasterize a Primitive into a pixel, we compute a Fragment.
	// It holds exactly the data needed by the fragmentStage to do depth tests,
	// shading, blending, and write to the framebuffer.
	struct Fragment {
		float depth;		// interpolated depth
		Color color;		// interpolated/perspective-corrected color
							// ... could add normal, UV, material ID, etc. here
	};
}
