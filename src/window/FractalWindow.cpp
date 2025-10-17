#include "FractalWindow.hpp"
#include <sstream>
#include "FractalImage.hpp"
#include <iostream>
#include <iomanip>
#include "fractal.hpp"

FractalWindow::FractalWindow(int width, int height, int n)
    : window_(nullptr), texture_(0), width_(width), height_(height),
      renderer_(width, height), view_{-5.0, -5.0, 5.0, 5.0, n},
      handCursor_(nullptr)
{
}

FractalWindow::~FractalWindow()
{
    if (handCursor_)
        glfwDestroyCursor(handCursor_);
    if (texture_)
        glDeleteTextures(1, &texture_);
    if (window_)
        glfwDestroyWindow(window_);
    glfwTerminate();
}

void FractalWindow::resize(int w, int h)
{
    width_ = w;
    height_ = h;
    renderer_.resize(w, h);
    glBindTexture(GL_TEXTURE_2D, texture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glViewport(0, 0, w, h);
    updateTexture();
}

void FractalWindow::handleInput()
{
    if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window_, true);

    double mouseX, mouseY;
    glfwGetCursorPos(window_, &mouseX, &mouseY);

    static double anchorWorldX = 0;
    static double anchorWorldY = 0;
    static bool isPanning = false;
    static bool wasPressed = false;

    // Panning
    bool isPressed = glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

    if (isPressed && !wasPressed)
    {
        double normalizedX = mouseX / width_;
        double normalizedY = mouseY / height_;
        anchorWorldX = view_.x0 + normalizedX * (view_.x1 - view_.x0);
        anchorWorldY = view_.y0 + normalizedY * (view_.y1 - view_.y0);
        isPanning = true;

        if (handCursor_)
            glfwSetCursor(window_, handCursor_);
    }

    if (isPressed && isPanning)
    {
        double normalizedX = mouseX / width_;
        double normalizedY = mouseY / height_;
        double currentWorldX = view_.x0 + normalizedX * (view_.x1 - view_.x0);
        double currentWorldY = view_.y0 + normalizedY * (view_.y1 - view_.y0);

        double deltaWorldX = anchorWorldX - currentWorldX;
        double deltaWorldY = anchorWorldY - currentWorldY;

        view_.x0 += deltaWorldX;
        view_.x1 += deltaWorldX;
        view_.y0 += deltaWorldY;
        view_.y1 += deltaWorldY;
    }

    if (!isPressed && wasPressed)
    {
        isPanning = false;
        glfwSetCursor(window_, nullptr);
    }

    wasPressed = isPressed;

    // Zoom with + and - keys
    auto zoomView = [&](double factor)
    {
        double cx = (view_.x0 + view_.x1) * 0.5;
        double cy = (view_.y0 + view_.y1) * 0.5;
        double rx = (view_.x1 - view_.x0) * factor * 0.5;
        double ry = (view_.y1 - view_.y0) * factor * 0.5;
        view_ = {cx - rx, cy - ry, cx + rx, cy + ry, view_.n};
    };

    if (glfwGetKey(window_, GLFW_KEY_EQUAL) == GLFW_PRESS)
        zoomView(0.9);
    if (glfwGetKey(window_, GLFW_KEY_MINUS) == GLFW_PRESS)
        zoomView(1.1);

    // Screenshot
    if (glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS)
    {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::ostringstream output;
        output << "fractal_" << std::put_time(&tm, "%Y%m%d_%H%M%S") << ".bmp";

        FractalImage image(width_, height_, view_);
        image.save(output.str());
    }

    // Change power and reset
    if (glfwGetKey(window_, GLFW_KEY_R) == GLFW_PRESS)
        view_ = {-5.0, -5.0, 5.0, 5.0, view_.n};
    for (int i = 2; i <= 9; ++i)
    {
        if (glfwGetKey(window_, GLFW_KEY_0 + i) == GLFW_PRESS)
            view_ = {-5.0, -5.0, 5.0, 5.0, i};
    }
}

void FractalWindow::scrollCallback(GLFWwindow* window, double, double yoffset)
{
    auto* pThis = static_cast<FractalWindow*>(glfwGetWindowUserPointer(window));
    pThis->handleScroll(yoffset);
}

void FractalWindow::handleScroll(double yoffset)
{
    double mouseX, mouseY;
    glfwGetCursorPos(window_, &mouseX, &mouseY);

    double normalizedX = mouseX / width_;
    double normalizedY = mouseY / height_;
    double worldMouseX = view_.x0 + normalizedX * (view_.x1 - view_.x0);
    double worldMouseY = view_.y0 + normalizedY * (view_.y1 - view_.y0);

    double zoomFactor = (yoffset > 0) ? 0.85 : 1.15;

    double newRangeX = (view_.x1 - view_.x0) * zoomFactor;
    double newRangeY = (view_.y1 - view_.y0) * zoomFactor;

    double ratioX = (worldMouseX - view_.x0) / (view_.x1 - view_.x0);
    double ratioY = (worldMouseY - view_.y0) / (view_.y1 - view_.y0);

    view_.x0 = worldMouseX - newRangeX * ratioX;
    view_.x1 = view_.x0 + newRangeX;
    view_.y0 = worldMouseY - newRangeY * ratioY;
    view_.y1 = view_.y0 + newRangeY;
}

void FractalWindow::updateTexture()
{
    renderer_.compute(view_);

    std::vector<uint32_t> pixels(width_ * height_);
    renderer_.getPixels(pixels.data());

    // Flip rows
    std::vector<uint32_t> flipped(width_ * height_);
    for (int y = 0; y < height_; ++y)
        for (int x = 0; x < width_; ++x)
            flipped[y * width_ + x] = pixels[(height_ - 1 - y) * width_ + x];

    glBindTexture(GL_TEXTURE_2D, texture_);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width_, height_, GL_RGBA, GL_UNSIGNED_BYTE, flipped.data());
}

void FractalWindow::framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    auto* pThis = static_cast<FractalWindow*>(glfwGetWindowUserPointer(window));
    pThis->resize(width, height);
}

void FractalWindow::render()
{
    drawTexturedFullscreen(texture_);
    glfwSwapBuffers(window_);
}

int FractalWindow::initialize()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to init GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window_ = glfwCreateWindow(width_, height_, "Fractal", nullptr, nullptr);
    if (!window_)
    {
        std::cerr << "Failed to create window\n";
        return -1;
    }

    glfwMakeContextCurrent(window_);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSetWindowUserPointer(window_, this);
    glfwSetFramebufferSizeCallback(window_, framebufferSizeCallback);
    glfwSetScrollCallback(window_, scrollCallback);
    glfwSwapInterval(0);
    glViewport(0, 0, width_, height_);

    if (!initShaderPipeline())
    {
        std::cerr << "Shader pipeline init failed\n";
        return -1;
    }

    handCursor_ = glfwCreateStandardCursor(GLFW_HAND_CURSOR);

    glGenTextures(1, &texture_);
    glBindTexture(GL_TEXTURE_2D, texture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    return 0;
}

int FractalWindow::run()
{
    if (initialize() != 0)
        return -1;

    while (!glfwWindowShouldClose(window_))
    {
        handleInput();
        render();
        updateTexture();
        glfwPollEvents();
    }

    shutdownShaderPipeline();
    return 0;
}