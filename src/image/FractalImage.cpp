#include "FractalImage.hpp"

FractalImage::FractalImage(int width, int height, const FractalRenderer::ViewParams& view) : width_(width), height_(height), view_(view)
{
}

bool FractalImage::save(const std::string& filename) const
{
	ExportParams params{
		width_,
		height_,
		view_,
		filename
	};
	return exportToBMP(params);
}
