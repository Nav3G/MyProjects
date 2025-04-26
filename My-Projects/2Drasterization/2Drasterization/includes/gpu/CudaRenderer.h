#pragma once
#include <vector>

#include "pipeline/PipelineTypes.h"				// for Pipeline::Primitive
#include "framework/Framebuffer.h"				// for Framebuffer

#include <cuda_runtime.h>
#include "gpu/RasterKernel.h"

class CudaRenderer
{
public:
	CudaRenderer(int width, int height, int maxPrims);
	~CudaRenderer();

	// Packs host-side Pipeline::Primitive into DevicePrimitive,
	// launches the two kernels, and copies back into fb.
	void render(const std::vector<Pipeline::Primitive>& prims,
		Framebuffer& fb);
private:
	int  W, H, maxPrimitives;

	// Device pointers: gpu-side framebuffers 
	DevicePrimitive* d_prims = nullptr;
	uchar3* d_color = nullptr;
	float* d_depth = nullptr;
};

