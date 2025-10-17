#include <string>
#include <iostream>
#include <glad/glad.h>

static GLuint gProgram = 0;
static GLuint gVAO = 0;
static GLuint gVBO = 0;
static GLint  gSamplerLoc = -1;

static constexpr const char* kVS = R"(#version 330 core
layout (location = 0) in vec2 inPos;
layout (location = 1) in vec2 inUV;
out vec2 vUV;
void main() {
    vUV = inUV;
    gl_Position = vec4(inPos, 0.0, 1.0);
}
)";

static constexpr const char* kFS = R"(#version 330 core
in vec2 vUV;
out vec4 FragColor;
uniform sampler2D uTex;
void main() {
    FragColor = texture(uTex, vUV);
}
)";

static GLuint compileShader(GLenum type, const char* src)
{
    GLuint sh = glCreateShader(type);
    glShaderSource(sh, 1, &src, nullptr);
    glCompileShader(sh);
    GLint ok = GL_FALSE;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        GLint len = 0;
        glGetShaderiv(sh, GL_INFO_LOG_LENGTH, &len);
        std::string log(len, '\0');
        glGetShaderInfoLog(sh, len, nullptr, log.data());
        std::cerr << "Shader compile error: " << log << std::endl;
        glDeleteShader(sh);
        return 0;
    }
    return sh;
}

static GLuint linkProgram(GLuint vs, GLuint fs)
{
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    GLint ok = GL_FALSE;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        GLint len = 0;
        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
        std::string log(len, '\0');
        glGetProgramInfoLog(prog, len, nullptr, log.data());
        std::cerr << "Program link error: " << log << std::endl;
        glDeleteProgram(prog);
        return 0;
    }
    return prog;
}

static void createFullscreenTriangle()
{
    const float verts[] = {
        -1.0f, -1.0f,      0.0f, 0.0f,
         3.0f, -1.0f,      2.0f, 0.0f,
        -1.0f,  3.0f,      0.0f, 2.0f,
    };

    glGenVertexArrays(1, &gVAO);
    glBindVertexArray(gVAO);

    glGenBuffers(1, &gVBO);
    glBindBuffer(GL_ARRAY_BUFFER, gVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0); // inPos
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (const void*)(0));

    glEnableVertexAttribArray(1); // inUV
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (const void*)(2 * sizeof(float)));

    glBindVertexArray(0);
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
        std::cerr << "GL error creating fullscreen triangle: " << err << std::endl;
}

[[nodiscard]] bool initShaderPipeline()
{
    GLuint vs = compileShader(GL_VERTEX_SHADER, kVS);
    if (!vs)
        return false;
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, kFS);
    if (!fs)
    {
        glDeleteShader(vs);
        return false;
    }

    gProgram = linkProgram(vs, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);
    if (!gProgram)
        return false;

    gSamplerLoc = glGetUniformLocation(gProgram, "uTex");

    createFullscreenTriangle();
    return gVAO != 0 && gProgram != 0;
}

void shutdownShaderPipeline()
{
    if (gVBO)
        glDeleteBuffers(1, &gVBO); gVBO = 0;
    if (gVAO)
        glDeleteVertexArrays(1, &gVAO); gVAO = 0;
    if (gProgram)
        glDeleteProgram(gProgram); gProgram = 0;
}

void drawTexturedFullscreen(GLuint texture)
{
    glUseProgram(gProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glBindVertexArray(gVAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}