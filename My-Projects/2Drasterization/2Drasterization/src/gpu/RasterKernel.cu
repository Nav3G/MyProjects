// RasterKernel.cu
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <cfloat>              // for FLT_MAX
#include "gpu/RasterKernel.h"

// In CUDA, everytime we launch a kernel with "myKernel<<<gridDim, blockDim>>>(…);"
// the CUDA runtime magically creates a two-level exectution space: Grid of thread 
// blocks, each block of threads. CUDA gives us three built-in integer vectors in 
// every kernel. blockIdx: the 3D index of *this* block within the grid, blockDim:
// the dimension (x,y,z) of *every* block, threadIdx: the 3D index of *this* thread
// within its block. Block size: each block is 16×16 threads, Grid size: enough blocks 
// so that grid.x*16 >= W and grid.y*16 => H. Each thread is responsible for is own pixel 
// calculations. So, once we launch the kernel and the grid is made, any work we need to do 
// on a given pixel (check, rasterize, ...) is executed in parallel with all other pixels.
// i.e. each thread runs this code at the same time. Read below...

// ---------------------------------------------------
// 1) clearBuffersKernel
// ---------------------------------------------------

// Parameters:
// __global__: Marks this as a kernel, i.e. a function that runs on the GPU
//  and is launched from the host via <<<...>>>. 
// uchar3* colorBuf: A pointer into GPU memory for the 2D color buffer, 
// laid out as a flat array of(R, G, B) bytes per pixel.
// float* depthBuff: A pointer into GPU memory for the 2D depth buffer, 
// one float per pixel.
// int W, H: The width and height of the fb.
// uchar3 bgColor: A packed(r, g, b) value for the background.
// float initDepth: The "infinite" depth value(we use FLT_MAX) for clearing
// Note that the pointer to buffers here are gpu sided for now.
//
// Thread indexing: 
// blockDim is the dimensions of each block (here dim3(16,16)), so blockDim.x == 16.
// blockIdx is the coordinates of this block in the 2D grid we launched.
// threadIdx is the coordinates of this thread within its block.
// Multiplying out gives each thread a unique (x,y) in the full image. e.g. x pos of 
// 6th block, 20th thread: so we are at the idx = 5th 16x16 block so thats, 5 * 16 x
// entries along. Then we just add the x coordinate of the thread we want xIdx = 
// 5 * 16 + 3 (3 since we are in the next row but 4 along) = column 83. Same for y.
//
// Bounds check
//
// Buffer indexing:
// Each thread is responsible for is own pixel calculations. They essentially each 
// execute this code block once. It picks out its associted pixel via the blockIdx,
// blockDim, and threadIdx which we've loaded as a buffer index. Then it just does 
// colorBuf[idx] = bgColor; depthBuf[idx] = initDepth; In this case, just fills the 
// associted pixel in the buffer with default values.
__global__ void clearBuffersKernel(
    uchar3* colorBuf,
    float* depthBuf,
    int     W,
    int     H,
    uchar3  bgColor,
    float   initDepth)
{
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    if (x >= W || y >= H) return;
    int idx = y * W + x;
    colorBuf[idx] = bgColor;
    depthBuf[idx] = initDepth;
}

// dim3: a struct that stores 3 dimension, default z = 1. We create
// a 16x16 "block" of threads. Then, we create a (width + 15)/16 x 
// (height + 15)/16 "grid" of blocks. We need enough blocks so that 
// grid.x * block.x >= width and similarly in Y. 
// (width + block.x - 1) / block.x is the standard integer “round up” trick. 
// E.g. if width=1200, (1200 + 15) / 16 = 76 blocks in X, each block covering 
// 16 pixels -> 76×16 = 1216 threads, of which the extra 16 simply do nothing 
// (we guard them in the kernel)
//
// <<<grid, block>>>: special CUDA launch operator recognized by nvcc. It tells 
// the driver: "please run grid.x * grid.y blocks, each with block.x * block.y 
// threads."
//
// cudaDeviceSynchronize(): makes the CPU block until all previously issued GPU work 
// is completed
void launchClearBuffers(
    uchar3* d_colorBuf,
    float* d_depthBuf,
    int     width,
    int     height)
{
    dim3 block(16, 16);
    dim3 grid((width + 15) / 16, (height + 15) / 16);
    clearBuffersKernel <<<grid, block>>> (d_colorBuf, d_depthBuf, 
        width, height, make_uchar3(150, 150, 150), FLT_MAX);
    cudaDeviceSynchronize();
}

// Everything is the same CUDA-wise. We are just executing this code 
// on every thread, but this time filling the device frame buffer with
// rasterized pixels.

// ---------------------------------------------------
// 2) rasterKernel stub
// ---------------------------------------------------
// edge function (2D cross-product) on the device
__device__ float edgeFn2D(float ax, float ay,
    float bx, float by,
    float px, float py)
{
    return (bx - ax) * (py - ay)
        - (by - ay) * (px - ax);
}

__global__ void rasterKernel(
    const DevicePrimitive* prims,
    int      numPrims,
    uchar3* colorBuf,
    float* depthBuf,
    int      W,
    int      H)
{
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    if (x >= W || y >= H) return;
    int idx = y * W + x;

    // start with the cleared values
    float bestZ = depthBuf[idx];
    uchar3 bestC = colorBuf[idx];

    // center of this pixel in screen coords
    float px = x + 0.5f, py = y + 0.5f;

    for (int i = 0; i < numPrims; ++i)
    {
        // load clip-space verts
        float4 c0 = prims[i].clipPos[0];
        float4 c1 = prims[i].clipPos[1];
        float4 c2 = prims[i].clipPos[2];

        // 1) perspective divide -> NDC
        float invW0 = 1.0f / c0.w;
        float invW1 = 1.0f / c1.w;
        float invW2 = 1.0f / c2.w;

        float3 ndc0 = make_float3(c0.x * invW0,
            c0.y * invW0,
            c0.z * invW0);
        float3 ndc1 = make_float3(c1.x * invW1,
            c1.y * invW1,
            c1.z * invW1);
        float3 ndc2 = make_float3(c2.x * invW2,
            c2.y * invW2,
            c2.z * invW2);

        // 2) NDC -> screen‐space
        float sx0 = (ndc0.x * 0.5f + 0.5f) * W;
        float sy0 = (1.0f - (ndc0.y * 0.5f + 0.5f)) * H;
        float sz0 = ndc0.z;

        float sx1 = (ndc1.x * 0.5f + 0.5f) * W;
        float sy1 = (1.0f - (ndc1.y * 0.5f + 0.5f)) * H;
        float sz1 = ndc1.z;

        float sx2 = (ndc2.x * 0.5f + 0.5f) * W;
        float sy2 = (1.0f - (ndc2.y * 0.5f + 0.5f)) * H;
        float sz2 = ndc2.z;

        // 3) premultiplied colors
        uchar3 col0 = prims[i].color[0];
        uchar3 col1 = prims[i].color[1];
        uchar3 col2 = prims[i].color[2];

        float r0 = col0.x * invW0, g0 = col0.y * invW0, b0 = col0.z * invW0;
        float r1 = col1.x * invW1, g1 = col1.y * invW1, b1 = col1.z * invW1;
        float r2 = col2.x * invW2, g2 = col2.y * invW2, b2 = col2.z * invW2;

        // 4) compute barycentrics
        float area = edgeFn2D(sx0, sy0, sx1, sy1, sx2, sy2);
        if (area == 0.0f) continue;

        float alpha = edgeFn2D(px, py, sx1, sy1, sx2, sy2) / area;
        float beta = edgeFn2D(px, py, sx2, sy2, sx0, sy0) / area;
        float gamma = 1.0f - alpha - beta;

        // 5) inside‐triangle test
        if (alpha < 0 || beta < 0 || gamma < 0) continue;

        // 6) perspective‐correct depth & color
        float oneOverW = alpha * invW0 + beta * invW1 + gamma * invW2;

        float z = (alpha * sz0 + beta * sz1 + gamma * sz2);

        // depth test (z already in NDC‐space, or we can divide by oneOverW)
        if (z < bestZ)
        {
            // interpolate color
            float r = (alpha * r0 + beta * r1 + gamma * r2) / oneOverW;
            float g = (alpha * g0 + beta * g1 + gamma * g2) / oneOverW;
            float b = (alpha * b0 + beta * b1 + gamma * b2) / oneOverW;

            bestZ = z;
            bestC = make_uchar3((uint8_t)r,
                (uint8_t)g,
                (uint8_t)b);
        }
    }

    // write out
    depthBuf[idx] = bestZ;
    colorBuf[idx] = bestC;
}

void launchRasterKernel(
    const DevicePrimitive* d_prims,
    int numPrims,
    uchar3* d_colorBuf,
    float* d_depthBuf,
    int     width,
    int     height)
{
    dim3 block(16, 16), grid((width + 15) / 16, (height + 15) / 16);
    rasterKernel <<<grid, block>>> (d_prims, numPrims,
        d_colorBuf, d_depthBuf,
        width, height);
    cudaDeviceSynchronize();
}
