#ifndef FRACTAL_EXPORTER_HPP
#define FRACTAL_EXPORTER_HPP

#include <string>
#include <vector>
#include <cstdint>
#include "FractalRenderer.hpp"

class FractalExporter
{
public:
	struct ExportParams
	{
		int width;
		int height;
		FractalRenderer::ViewParams view;
		std::string filename;
	};

	static bool exportToBMP(const ExportParams& params);

protected:
	static std::vector<uint32_t> computeFractal(int width, int height, const FractalRenderer::ViewParams& view);
};

#endif