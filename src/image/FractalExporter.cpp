#include "FractalExporter.hpp"
#include "fractal.hpp"
#include <iostream>
#include <fstream>
#include <cstring>

std::vector<uint32_t> FractalExporter::computeFractal(int width, int height, const FractalRenderer::ViewParams& view)
{
    FractalRenderer         renderer(width, height);
    std::vector<uint32_t>   pixels(width * height);

    renderer.compute(view);
    renderer.getPixels(pixels.data());

    return pixels;
}

bool FractalExporter::exportToBMP(const ExportParams& params)
{
    std::cout << "Computing fractal " << params.width << "x" << params.height << " for export..." << std::endl;

    auto pixels = computeFractal(params.width, params.height, params.view);

    std::cout << "Writing BMP to " << params.filename << std::endl;

    std::ofstream file(params.filename, std::ios::binary);
    if (!file)
    {
        std::cerr << "Failed to open file for writing: " << params.filename << std::endl;
        return false;
    }

    int width = params.width;
    int height = params.height;

    // BMP Header (14 bytes)
    struct BMPFileHeader {
        uint16_t signature;
        uint32_t fileSize;
        uint16_t reserved1;
        uint16_t reserved2;
        uint32_t pixelDataOffset;
    } fileHeader;

    fileHeader.signature = 0x4D42;  // 'BM'
    fileHeader.pixelDataOffset = 54;  // 14 (file header) + 40 (info header)
    fileHeader.fileSize = fileHeader.pixelDataOffset + (width * height * 4);
    fileHeader.reserved1 = 0;
    fileHeader.reserved2 = 0;

    // BMP Info Header (40 bytes)
    struct BMPInfoHeader {
        uint32_t headerSize;
        int32_t width;
        int32_t height;
        uint16_t planes;
        uint16_t bitsPerPixel;
        uint32_t compression;
        uint32_t imageSize;
        int32_t pixelsPerMeterX;
        int32_t pixelsPerMeterY;
        uint32_t colorsUsed;
        uint32_t colorsImportant;
    } infoHeader;

    infoHeader.headerSize = 40;
    infoHeader.width = width;
    infoHeader.height = height;
    infoHeader.planes = 1;
    infoHeader.bitsPerPixel = 32;
    infoHeader.compression = 0;
    infoHeader.imageSize = width * height * 4;
    infoHeader.pixelsPerMeterX = 2835;
    infoHeader.pixelsPerMeterY = 2835;
    infoHeader.colorsUsed = 0;
    infoHeader.colorsImportant = 0;

    // Write file header
    file.write((char*)&fileHeader.signature, 2);
    file.write((char*)&fileHeader.fileSize, 4);
    file.write((char*)&fileHeader.reserved1, 2);
    file.write((char*)&fileHeader.reserved2, 2);
    file.write((char*)&fileHeader.pixelDataOffset, 4);

    // Write info header
    file.write((char*)&infoHeader.headerSize, 4);
    file.write((char*)&infoHeader.width, 4);
    file.write((char*)&infoHeader.height, 4);
    file.write((char*)&infoHeader.planes, 2);
    file.write((char*)&infoHeader.bitsPerPixel, 2);
    file.write((char*)&infoHeader.compression, 4);
    file.write((char*)&infoHeader.imageSize, 4);
    file.write((char*)&infoHeader.pixelsPerMeterX, 4);
    file.write((char*)&infoHeader.pixelsPerMeterY, 4);
    file.write((char*)&infoHeader.colorsUsed, 4);
    file.write((char*)&infoHeader.colorsImportant, 4);

    // Write pixel data
    for (int y = height - 1; y >= 0; --y)
    {
        for (int x = 0; x < width; ++x)
        {
            uint32_t pixel = pixels[y * width + x];
            uint8_t r = (pixel >> 0) & 0xFF;
            uint8_t g = (pixel >> 8) & 0xFF;
            uint8_t b = (pixel >> 16) & 0xFF;
            uint8_t a = (pixel >> 24) & 0xFF;

            file.write((char*)&b, 1);
            file.write((char*)&g, 1);
            file.write((char*)&r, 1);
            file.write((char*)&a, 1);
        }
    }

    file.close();

    std::cout << "Export complete!" << std::endl;
    return true;
}