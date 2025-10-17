#ifndef FRACTAL_IMAGE_HPP
#define FRACTAL_IMAGE_HPP

#include "FractalExporter.hpp"

class FractalImage : public FractalExporter
{
public:
	FractalImage(int width, int height, const FractalRenderer::ViewParams& view);

	bool save(const std::string& filename) const;

private:
	int width_;
	int height_;
	FractalRenderer::ViewParams view_;
};

#endif