#ifndef FRACTOL_H
# define FRACTOL_H

#include <glad/glad.h>

// Macros
# define WIN_WIDTH 1111
# define WIN_HEIGHT 1111
# define MAX_ITERATIONS 2000

// Shaders
bool initShaderPipeline();
void shutdownShaderPipeline();
void drawTexturedFullscreen(GLuint texture);

#endif
