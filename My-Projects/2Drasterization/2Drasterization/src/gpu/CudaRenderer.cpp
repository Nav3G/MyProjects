#include "gpu/CudaRenderer.h"
#include "gpu/RasterKernel.h"
#include <cuda_runtime.h>
#include <stdexcept>

CudaRenderer::CudaRenderer(int width, int height, int maxPrims)
    : W(width), H(height), maxPrimitives(maxPrims)
{
    // 1) Allocate once on the GPU
    size_t primBytes = size_t(maxPrimitives) * sizeof(DevicePrimitive);
    cudaMalloc(&d_prims, primBytes);

    size_t bufSize = size_t(W) * H;
    cudaMalloc(&d_color, bufSize * sizeof(uchar3));
    cudaMalloc(&d_depth, bufSize * sizeof(float));
}

CudaRenderer::~CudaRenderer()
{
    cudaFree(d_prims);
    cudaFree(d_color);
    cudaFree(d_depth);
}

void CudaRenderer::render(
    const std::vector<Pipeline::Primitive>& prims,
    Framebuffer& fb)
{
    int numPrims = int(prims.size());
    if (numPrims > maxPrimitives)
        throw std::runtime_error("Too many primitives for CUDA buffer");

    // 2) Pack host -> device
    std::vector<DevicePrimitive> hostPrims(numPrims);
    for (int i = 0; i < numPrims; ++i) {
        for (int j = 0; j < 3; ++j) {
            auto& v = prims[i][j];
            hostPrims[i].clipPos[j] = make_float4(
                v.clipPos.x(), v.clipPos.y(),
                v.clipPos.z(), v.clipPos.w());
            hostPrims[i].color[j] = make_uchar3(
                v.color.r, v.color.g, v.color.b);
        }
    }
    cudaMemcpy(d_prims, hostPrims.data(),
        numPrims * sizeof(DevicePrimitive), cudaMemcpyHostToDevice);

    // 3) Clear, raster, then copy back
    launchClearBuffers(d_color, d_depth, W, H);
    launchRasterKernel(d_prims, numPrims, d_color, d_depth, W, H);

    // Copy final color buffer into CPU-side framebuffer
    cudaMemcpy(fb.getColorBuffer().data(),
        d_color,
        size_t(W) * H * sizeof(uchar3),
        cudaMemcpyDeviceToHost);
}

