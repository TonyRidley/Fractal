#ifndef FRACTAL_WINDOW_HPP
#define FRACTAL_WINDOW_HPP

#ifdef __APPLE_
#else
    #include <glad/glad.h>
#endif
#include <GLFW/glfw3.h>
#include "FractalRenderer.hpp"

class FractalWindow
{
public:
    FractalWindow(int width, int height, int n);
    ~FractalWindow();

    int run();

private:
    GLFWwindow* window_;
    GLuint texture_;
    int width_, height_;

    FractalRenderer renderer_;
    FractalRenderer::ViewParams view_;
    GLFWcursor *handCursor_;

    void handleInput();
    void updateTexture();
    void render();
    void handleScroll(double yoffset);
    void resize(int w, int h);
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    int initialize();
};

#endif