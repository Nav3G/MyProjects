#include "gpu/CudaRenderer.h"
#include "gpu/RasterKernel.h"
#include <cuda_runtime.h>
#include <stdexcept>

inline int clampI(int x, int lo, int hi) {
    return x < lo ? lo : (x > hi ? hi : x);
}

// Constructor 
// W, H, maxPrimitives: Stores the framebuffer dimensions and the maximum 
// number of triangles we ever batch.
// cudaMalloc(&d_prims, primBytes): Allocates enough GPU memory for triangle
// data. This call takes a pointer to the allocated items in memory and the number
// of bytes.
// cudaMalloc(&d_color, bufsize * sizeof(ucahr3)): Reserves W×H pixels worth 
// of 3-byte colors.
// cudaMalloc(&d_depth, bufSize * sizeof(float)): Reserves W×H floats for depth.
//
// cudaMalloc(&d_cellOffsets_, sizeof(int) * (numTiles + 1)) :We also allocate a 
// tile-offset array of length numTilesX*numTilesY + 1
CudaRenderer::CudaRenderer(int width, int height, int maxPrims)
    : W(width), H(height), maxPrimitives(maxPrims)
{
    // 1) Allocate once on the GPU
    size_t primBytes = size_t(maxPrimitives) * sizeof(DevicePrimitive);
    cudaMalloc(&d_prims, primBytes);

    size_t bufSize = size_t(W) * H;
    cudaMalloc(&d_color, bufSize * sizeof(uchar3));
    cudaMalloc(&d_depth, bufSize * sizeof(float));

    // 2) Allocate the maximum‐possible tile‐offset array once
    const int tileSize = 16;
    int Gx = (W + tileSize - 1) / tileSize;
    int Gy = (H + tileSize - 1) / tileSize;
    int numTiles = Gx * Gy;
    cudaMalloc(&d_cellOffsets_, sizeof(int) * (numTiles + 1));

    // 3) ONE-TIME host allocation
    hostPrims_.reserve(maxPrimitives);
}

// Desructor
// Frees memory allocated to buffers.
CudaRenderer::~CudaRenderer()
{
    cudaFree(d_prims);
    cudaFree(d_color);
    cudaFree(d_depth);

    cudaFree(d_cellOffsets_);
    cudaFree(d_cellTriIndices_);
}

// Cell list builder 
// We want to spatially partition each triangle into a 2D grid of tiles, so each pixel
// only tests the handful of triangles that actually overlap its tile instead of all 
// of them. It does two passes: A counting pass, A prefix-sum and fill pass.
// Count pass: We project each triangle into spreen coordinates like usual. Then, we 
// compute its axis-aligned pixel bounding box and clamp it, again like usual. Then, 
// we convert those pixel bounds to tile bounds but integer-dividing by tilesize. Then,
// for every tile in that range, we increment the counter. 
// Prefix-sum and fill pass: First we turn the per-tile counters into a prefix-sum array
// so that cellOffsets[t] = the sum over the count of tirangles in tile k for k < t. We 
// then allocate a flat index array (cellTriIndices) of total size cellOffsetsets[last].
// Again, we iterate through each triangle, and for each tile they overlap, write their 
// ID into the next free slot in that tile's range (tracked via cursor array).
//
// cellOffsets: A std::vector<int> of size numTiles + 1. For each tile t (0 <= t < 
// numTiles), cellOffsets[t] tells us where in the flat index array we can find
// that tile's triangle list. The extra slot at cellOffsets[numTiles] hold the total
// count of triangle-tile overlaps (so the last tile's list runs from 
// cellOffsets[numTiles  - 1] up to but not including cellOffset[numTiles].
//
// cellTriIndices: A std::vector<int>. This is a flat list of triangle indices. Triangles
// are grouped by tile: tile 0's list = cellTriIndices[ cellOffsets[0] ... cellOffsets[1]-1]
// tile 1's list = cellTriIndices[cellOffsets[1]... cellOffsets[2] - 1] ... and so on.
// 
// cursor: A std::vector<int>, local. A temporary copy of cellOffsets used in the secnod pass
// to track where to write the next triangle index for each tile.
void buildCellLists(const std::vector<DevicePrimitive>& hostPrims, int W, int H, 
    int tileSize, std::vector<int>& cellOffsets, std::vector<int>& cellTriIndices)
{
    // 1) Compute grid dimensions
    int Gx = (W + tileSize - 1) / tileSize;
    int Gy = (H + tileSize - 1) / tileSize;
    int numTiles = Gx * Gy;
    int N = (int)hostPrims.size();

    cellOffsets.assign(numTiles + 1, 0);

    // 2) For each triangle...
    for (int i = 0; i < N; i++)
    {
        // Project its 3 clip-space verts into pixel-space
        float sx[3], sy[3];
        for (int v = 0; v < 3; ++v) 
        {
            const float4& P = hostPrims[i].clipPos[v];
            float invW = 1.0f / P.w;
            // NDC
            float ndcX = P.x * invW;
            float ndcY = P.y * invW;
            // To screen
            sx[v] = (ndcX * 0.5f + 0.5f) * W;
            sy[v] = (1.0f - (ndcY * 0.5f + 0.5f)) * H;
        }

        // Compute pixel-space bbox
        float fx0 = std::min({ sx[0], sx[1], sx[2] });
        float fx1 = std::max({ sx[0], sx[1], sx[2] });
        float fy0 = std::min({ sy[0], sy[1], sy[2] });
        float fy1 = std::max({ sy[0], sy[1], sy[2] });

        int minX = std::max(0, int(std::floor(fx0)));
        int maxX = std::min(W - 1, int(std::ceil(fx1)));
        int minY = std::max(0, int(std::floor(fy0)));
        int maxY = std::min(H - 1, int(std::ceil(fy1)));

        // Convert to tile indices
        int tminX = minX / tileSize;
        int tmaxX = maxX / tileSize;
        int tminY = minY / tileSize;
        int tmaxY = maxY / tileSize;

        // Clamp to grid
        tminX = std::max(0, std::min(tminX, Gx - 1));
        tmaxX = std::max(0, std::min(tmaxX, Gx - 1));
        tminY = std::max(0, std::min(tminY, Gy - 1));
        tmaxY = std::max(0, std::min(tmaxY, Gy - 1));

        // For each tile that this tri overlaps, bump the count
        for (int ty = tminY; ty <= tmaxY; ++ty) {
            int base = ty * Gx;
            for (int tx = tminX; tx <= tmaxX; ++tx) {
                cellOffsets[base + tx + 1]++;
            }
        }
    }

    // Now we turn each count into a prefix-sum
    for (int t = 1; t <= numTiles; ++t)
    {
        cellOffsets[t] += cellOffsets[t - 1];
    }

    cellTriIndices.resize(cellOffsets[numTiles]);
    std::vector<int> cursor = cellOffsets;

    for (int i = 0; i < N; ++i) {
        // same projection & tile‐range compute as above
        float sx[3], sy[3];
        for (int v = 0; v < 3; ++v) {
            const float4& P = hostPrims[i].clipPos[v];
            float invW = 1.0f / P.w;
            float ndcX = P.x * invW, ndcY = P.y * invW;
            sx[v] = (ndcX * 0.5f + 0.5f) * W;
            sy[v] = (1.0f - (ndcY * 0.5f + 0.5f)) * H;
        }
        int minX = std::max(0, int(std::floor(std::min({ sx[0],sx[1],sx[2] }))));
        int maxX = std::min(W - 1, int(std::ceil(std::max({ sx[0],sx[1],sx[2] }))));
        int minY = std::max(0, int(std::floor(std::min({ sy[0],sy[1],sy[2] }))));
        int maxY = std::min(H - 1, int(std::ceil(std::max({ sy[0],sy[1],sy[2] }))));
        int tminX = clampI(minX / tileSize, 0, Gx - 1);
        int tmaxX = clampI(maxX / tileSize, 0, Gx - 1);
        int tminY = clampI(minY / tileSize, 0, Gy - 1);
        int tmaxY = clampI(maxY / tileSize, 0, Gy - 1);

        for (int ty = tminY; ty <= tmaxY; ++ty) {
            int base = ty * Gx;
            for (int tx = tminX; tx <= tmaxX; ++tx) {
                int tileIdx = base + tx;
                int writePosition = cursor[tileIdx]++;
                cellTriIndices[writePosition] = i;
            }
        }
    }
}


// Renderer
// Per-frame data upload, launch, download. 0) Bounds check: Making sure
// to never overflow the triangle buffer we allocated. 1) Packing array 
// of DevicePrimitives with host primitive data. 2) Fill host cellOffsets
// and cellTriIndices. 3) Upload those two arrays and the DivicePrimitives
// into GPU memory. 4) LaunchClearBuffer to reset 5) LaunchRasterPixels to 
// draw. 6) Tell CPU to wait 7) Download to host: Copies device framebuffer 
// memory back to CPU framebuffer.
void CudaRenderer::render(
    const std::vector<Pipeline::Primitive>& prims,
    Framebuffer& fb)
{
    // 0) Bounds check
    int numPrims = int(prims.size());
    if (numPrims > maxPrimitives)
        throw std::runtime_error("Too many primitives");

    // 1) Pack hostPrims_
    hostPrims_.resize(numPrims);
    for (int i = 0; i < numPrims; ++i) {
        auto& dst = hostPrims_[i];
        for (int v = 0; v < 3; ++v) {
            auto& src = prims[i][v];
            dst.clipPos[v] = make_float4(
                src.clipPos.x(),
                src.clipPos.y(),
                src.clipPos.z(),
                src.clipPos.w()
            );
            dst.color[v] = make_uchar3(
                src.color.r,
                src.color.g,
                src.color.b
            );
        }
    }

    // 2) Build & upload cell‐lists
    const int tileSize = 16;
    buildCellLists(hostPrims_, W, H, tileSize, hostCellOffsets_, hostCellTriIndices_);

    // Upload offsets. Convert the host offset data to device data.
    cudaMemcpy(d_cellOffsets_, hostCellOffsets_.data(), sizeof(int) * hostCellOffsets_.size(),
        cudaMemcpyHostToDevice);

    // (Re)allocate & upload indices. Convert the host cell data to device data.
    if (d_cellTriIndices_) cudaFree(d_cellTriIndices_);
    cudaMalloc(&d_cellTriIndices_, sizeof(int) * hostCellTriIndices_.size());
    cudaMemcpy(d_cellTriIndices_, hostCellTriIndices_.data(), 
        sizeof(int) * hostCellTriIndices_.size(), cudaMemcpyHostToDevice);

    // 3) Upload primitives. Convert the host primitive data to device data.
    cudaMemcpy(d_prims, hostPrims_.data(), sizeof(DevicePrimitive) * numPrims,
        cudaMemcpyHostToDevice);

    // 4) Clear
    launchClearBuffers(d_color, d_depth, W, H);

    // 5) Rasterize: one thread per pixel, but only tri‐list for its tile
    launchRasterPixels(d_prims, numPrims, d_cellOffsets_, d_cellTriIndices_, d_color, d_depth, W, H,
        tileSize, (W + tileSize - 1) / tileSize);

    // 6) Wait
    cudaDeviceSynchronize();

    // 7) Download color buffer
    cudaMemcpy(fb.getColorBuffer().data(), d_color, sizeof(uchar3) * size_t(W) * H,
        cudaMemcpyDeviceToHost);
}