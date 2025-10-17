#ifndef FRACTAL_RENDERER_HPP
#define FRACTAL_RENDERER_HPP

#include <vector>
#include <cstdint>
#include "ThreadPool.hpp"

class FractalRenderer
{
public:
	struct ViewParams
	{
		double	x0, y0, x1, y1;
		int		n;
	};

	FractalRenderer(int width, int height);
	~FractalRenderer();

	void resize(int width, int height);
	void compute(const ViewParams& view);
	void getPixels(uint32_t* outPixels) const;

	int getWidth() const { return width_; }
	int getHeight() const { return height_; }

private:
	int width_;
	int height_;
	std::vector<uint32_t> pixels_;
	ThreadPool threadPool_;
	static constexpr int TILE_SIZE = 64;

	void computeTile_(int tileX, int tileY, const ViewParams& view);
};

#endif