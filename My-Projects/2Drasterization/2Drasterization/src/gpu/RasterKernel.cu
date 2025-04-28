// RasterKernel.cu
#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include "gpu/RasterKernel.h"

// Raster Helpers
__device__ float edgeFn(const float2 a, const float2 b, const float2 p) {
    return (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
}

__device__ bool insideTriangle(
    const float2 s0, const float2 s1, const float2 s2,
    const float2 p, float& alpha, float& beta, float& gamma)
{
    float area = edgeFn(s0, s1, s2);
    alpha = edgeFn(p, s1, s2) / area;
    beta = edgeFn(p, s2, s0) / area;
    gamma = edgeFn(p, s0, s1) / area;
    return (alpha >= 0 && beta >= 0 && gamma >= 0);
}

__device__ float interpDepth(
    float z0, float z1, float z2,
    float alpha, float beta, float gamma)
{
    return alpha * z0 + beta * z1 + gamma * z2;
}

__device__ uchar3 interpColorPC(
    const uchar3 c0, const uchar3 c1, const uchar3 c2,
    const float invW0, const float invW1, const float invW2,
    float alpha, float beta, float gamma)
{
    float oW = alpha * invW0 + beta * invW1 + gamma * invW2;
    float r = (alpha * c0.x * invW0 + beta * c1.x * invW1 + gamma * c2.x * invW2) / oW;
    float g = (alpha * c0.y * invW0 + beta * c1.y * invW1 + gamma * c2.y * invW2) / oW;
    float b = (alpha * c0.z * invW0 + beta * c1.z * invW1 + gamma * c2.z * invW2) / oW;
    return make_uchar3(r, g, b);
}

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
// Bounds check: self explanatory
//
// Buffer indexing:
// Each thread is responsible for is own pixel calculations. They essentially each 
// execute this code block once. It picks out its associted pixel via the blockIdx,
// blockDim, and threadIdx which we've loaded as a buffer index. Then it just does 
// colorBuf[idx] = bgColor; depthBuf[idx] = initDepth; In this case, just fills the 
// associted pixel in the buffer with default values.
__global__ void clearBuffersKernel(
    uchar3* d_colorBuf,
    float* d_depthBuf,
    int     W,
    int     H,
    uchar3  bgColor,
    float   initDepth)
{
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    if (x >= W || y >= H) return;
    int idx = y * W + x;
    d_colorBuf[idx] = bgColor;
    d_depthBuf[idx] = initDepth;
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
// cudaDeviceSynchronize(): makes the CPU ignore until all previously issued GPU work 
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

// Rasterization
// For every pixel, concurrently, we test only the triangles touching that pixel's
// tile, then interpolate depth and color, and lastly write the closest fragments.
// For a pixel at (x,y): We start with a bounds check and skip if oustside. Next, 
// we do a tile lookup by dividing the pixel coordinate by tilesize and defining 
// that tile. This is the tile that pixel (x,y) lives in. Now we loop through each 
// triangle overlapping the given tile. We fetch its vertices' screen position as well 
// as the precopmuted clip-space interpolation data. Then we compute barycentrics, 
// interpolate depth, test depth, and write like usual.
static __global__
void rasterPixelsKernel(const DevicePrimitive* prims, int numPrims,
    const int* cellOffsets, const int* cellTriIndices,
        uchar3* outColor, float* outDepth, 
            int W, int H,
                int tileSize, int numTilesX,
                    uchar3 bgColor, float initDepth)
{
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    if (x >= W || y >= H) return;

    int idx = y * W + x;
    // Seed with clear values
    float bestDepth = outDepth[idx];
    uchar3 bestColor = outColor[idx];

    // Determine tile containing pixel (x,y) and its triangle list
    int tx = x / tileSize;
    int ty = y / tileSize;
    int cell = ty * numTilesX + tx;     // Which tile pixel (x,y) lives in
    int start = cellOffsets[cell];      // The list of triangle IDs that might cover any pixel in cell
    int end = cellOffsets[cell + 1];

    // Test point
    float2 p = make_float2(x + 0.5f, y + 0.5f);

    // Loop ONLY the candidate triangles overlapping this tile (cell)
    for (int ptr = start; ptr < end; ++ptr) {
        int triID = cellTriIndices[ptr];
        const DevicePrimitive& tri = prims[triID];       // The triangle overlapping this tile

        // Screen-space data temp holders
        float2 s[3];            // x,y position of 3 vertices in screen-space
        float  zs[3], invW[3];  // z depth and inverse of ws for 3 vertices
        uchar3 col[3];          // Colors atribured to each vertex

        // Looping through each vertex of test triangle to determine screen-space data
        for (int v = 0; v < 3; ++v) {
            float4 P = tri.clipPos[v];      // Clip-space point vertex
            float wInv = 1.0f / P.w;        // 1/w
            invW[v] = wInv;                 

            // NDC
            float ndcX = P.x * wInv;        // Perspecive divides to NDC             
            float ndcY = P.y * wInv;            
            zs[v] = P.z * wInv;             // post-divide depth

            // toScreen
            s[v].x = (ndcX * 0.5f + 0.5f) * W;
            s[v].y = (1.0f - (ndcY * 0.5f + 0.5f)) * H;

            col[v] = tri.color[v];
        }

        // Inside-triangle test + barycentrics
        float alpha, beta, gamma;
        if (!insideTriangle(s[0], s[1], s[2], p, alpha, beta, gamma)) continue;

        // Depth interpolation and test
        float d = interpDepth(zs[0], zs[1], zs[2], alpha, beta, gamma);
        if (d < bestDepth) {
            bestDepth = d;
            bestColor = interpColorPC(col[0], col[1], col[2], invW[0], invW[1], invW[2], alpha, beta, gamma);
        }
    }

    // 5) write out
    outDepth[idx] = bestDepth;
    outColor[idx] = bestColor;
}

// Launcher that passes clear params and calls kernel
void launchRasterPixels(
    const DevicePrimitive* d_prims,
    int                    numPrims,
    const int* d_cellOffsets,
    const int* d_cellTriIndices,
    uchar3* d_colorBuf,
    float* d_depthBuf,
    int                    width,
    int                    height,
    int                    tileSize,
    int                    numTilesX)
{
    // Build thread grid
    dim3 block(16, 16);
    dim3 grid((width + 15) / 16, (height + 15) / 16);
    uchar3 bg = make_uchar3(150, 150, 150);
    float  id = FLT_MAX;
    
    // Call rasterization kernel
    rasterPixelsKernel <<<grid, block >>> (d_prims, numPrims, d_cellOffsets, 
        d_cellTriIndices, d_colorBuf, d_depthBuf, width, height, tileSize, numTilesX,
            bg, id);

    // Tell CPU to wait
    cudaDeviceSynchronize();
}