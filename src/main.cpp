#include <iomanip>
#include <iostream>
#include <sstream>
#include <cstring>
#include "fractal.hpp"
#include "FractalImage.hpp"
#include "FractalWindow.hpp"

static int	input_validation(int argc, char *argv[]);
static void	print_usage();

int main(int argc, char* argv[])
{
	int	n;
	n = input_validation(argc, argv);

	if (argc == 3)
	{
		auto t = std::time(nullptr);
		auto tm = *std::localtime(&t);
		FractalRenderer::ViewParams view{-5.0, -5.0, 5.0, 5.0, n};
		FractalImage image(WIN_WIDTH, WIN_HEIGHT, view);

		std::ostringstream output;
		output << "fractal_" << std::put_time(&tm, "%Y%m%d_%H%M%S") << ".bmp";
		if (image.save(output.str()))
			return 0;
		return 1;
	}

	FractalWindow window(WIN_WIDTH, WIN_HEIGHT, n);
	return window.run();
}

static int input_validation(int argc, char *argv[])
{
	char	*end;
	long	n;

	if (argc != 2 && argc != 3)
	{
		print_usage();
		exit(-1);
	}
	if (argc == 3)
	{
		if (std::strcmp(argv[2], "-i") != 0 && std::strcmp(argv[2], "--image") != 0)
		{
			print_usage();
			exit(-1);
		}
	}

	n = std::strtol(argv[1], &end, 10);

	if (end == argv[1])
	{
		std::cerr << "N missing or not a number" << std::endl;
		exit(-1);
	}
	if (*end != '\0')
	{
		std::cerr << "N has extra characters" << std::endl;
		exit(-1);
	}
	if (n < 2 || n > 50)
	{
		std::cerr << "N must be an integer in [2,50]" << std::endl;
		exit(-1);
	}

	return static_cast<int>(n);
}

static void print_usage()
{
	std::cout << "Usage: ./Fractal [OPTIONS]\n"
			  << "  (no args)     Run interactive window\n"
			  << "  --image       Save a default fractal image\n"
			  << "  --help        Show this message\n";
}