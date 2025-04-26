#pragma once
// CUDA runtime API (defines cudaMalloc, cudaMemcpy, float4, uchar3, etc.)
#include <cuda_runtime.h>

// Defines the built-in variables blockIdx, threadIdx, etc.
#include <device_launch_parameters.h>

// FLT_MAX for an "infinite" depth clear
#include <cfloat>

// Use uint8_t / uchar3 etc.
#include <cstdint>

// DevicePrimitive: 3 clip-space positions + colors
struct DevicePrimitive {
    float4 clipPos[3];      // x,y,z,w
    uchar3 color[3];        // r,g,b
};

// ScreenTriangle: after divide + toScreen (in host), upload these to device
struct DeviceScreenTri {
    float3 s[3];            // screen-space xyz
    float invW[3];          // 1/w
    float rOverW[3], gOverW[3], bOverW[3];
};

// Host-callable wrappers (implemented in RasterKernel.cu)
void launchClearBuffers(uchar3* d_colorBuf,
    float* d_depthBuf,
    int     width,
    int     height);

void launchRasterKernel(const DevicePrimitive* d_prims,
    int numPrims,
    uchar3* d_colorBuf,
    float* d_depthBuf,
    int     width,
    int     height);