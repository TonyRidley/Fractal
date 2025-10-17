#include "FractalRenderer.hpp"

#include <thread>
#include <future>
#include <algorithm>
#include <numeric>
#include "fractal.hpp"
#include "fractal_ispc.h"

FractalRenderer::FractalRenderer(int width, int height)
    : width_(width), height_(height), pixels_(width * height),
      threadPool_(std::thread::hardware_concurrency())
{
}

FractalRenderer::~FractalRenderer()
{
}

void FractalRenderer::resize(int width, int height)
{
    width_ = width;
    height_ = height;
    pixels_.resize(width * height);
}

void FractalRenderer::compute(const ViewParams& view)
{
    const int tilesX = (width_ + TILE_SIZE - 1) / TILE_SIZE;
    const int tilesY = (height_ + TILE_SIZE - 1) / TILE_SIZE;
    const int totalTiles = tilesX * tilesY;

    std::vector<std::future<void>> futures;
    futures.reserve(totalTiles);

    for (int ty = 0; ty < tilesY; ++ty)
    {
        for (int tx = 0; tx < tilesX; ++tx)
        {
            futures.push_back(threadPool_.enqueue([this, tx, ty, &view]() {
                this->computeTile_(tx, ty, view);
            }));
        }
    }

    for (auto& future : futures)
        future.wait();
}

void FractalRenderer::getPixels(uint32_t* outPixels) const
{
    std::copy(pixels_.begin(), pixels_.end(), outPixels);
}

void FractalRenderer::computeTile_(int tileX, int tileY, const ViewParams& view)
{
    const int startX = tileX * TILE_SIZE;
    const int startY = tileY * TILE_SIZE;
    const int endX = std::min(startX + TILE_SIZE, width_);
    const int endY = std::min(startY + TILE_SIZE, height_);

    const int tileWidth = endX - startX;
    const int tileHeight = endY - startY;

    const double inv_width = 1.0 / width_;
    const double inv_height = 1.0 / height_;
    const double rangeX = view.x1 - view.x0;
    const double rangeY = view.y1 - view.y0;

    const double tileX0 = view.x0 + startX * rangeX * inv_width;
    const double tileY0 = view.y0 + startY * rangeY * inv_height;
    const double tileX1 = view.x0 + endX * rangeX * inv_width;
    const double tileY1 = view.y0 + endY * rangeY * inv_height;

    uint32_t* tileStart = pixels_.data() + (startY * width_ + startX);

    ispc::compute_newton_fractal_rgba32(
        tileStart,
        tileWidth,
        tileHeight,
        width_,
        static_cast<float>(tileX0),
        static_cast<float>(tileY0),
        static_cast<float>(tileX1),
        static_cast<float>(tileY1),
        MAX_ITERATIONS,
        view.n
    );
}