#include "pch.h"
#include "gdInternal.h"

#ifdef _WIN32
    #pragma comment(lib, "opengl32.lib")
    //#pragma comment(lib, "glew32s.lib")
    #define GLEW_STATIC
#endif
#include <GL/glew.h>

class GraphicsDeviceOpenGL : public GraphicsDevice
{
public:
    GraphicsDeviceOpenGL();
    ~GraphicsDeviceOpenGL();
    void* getDevicePtr() override;
    GraphicsDeviceType getDeviceType() override;
    void sync() override;

    bool readTexture(void *o_buf, size_t bufsize, void *tex, int width, int height, PixelFormat format) override;
    bool writeTexture(void *o_tex, int width, int height, PixelFormat format, const void *buf, size_t bufsize) override;

    bool readBuffer(void *dst, const void *src_buf, size_t srcsize) override;
    bool writeBuffer(void *dst_buf, const void *src, size_t srcsize) override;
};


GraphicsDevice* CreateGraphicsDeviceOpenGL()
{
    return new GraphicsDeviceOpenGL();
}


void* GraphicsDeviceOpenGL::getDevicePtr() { return nullptr; }
GraphicsDeviceType GraphicsDeviceOpenGL::getDeviceType() { return GraphicsDeviceType::OpenGL; }

GraphicsDeviceOpenGL::GraphicsDeviceOpenGL()
{
//    glewInit();
}

GraphicsDeviceOpenGL::~GraphicsDeviceOpenGL()
{
}


static void GetInternalFormatOpenGL(PixelFormat format, GLenum &o_fmt, GLenum &o_type)
{
    switch (format)
    {
    case PixelFormat::RGBAu8:   o_fmt = GL_RGBA; o_type = GL_UNSIGNED_BYTE; return;

    case PixelFormat::RGBAf16:  o_fmt = GL_RGBA; o_type = GL_HALF_FLOAT; return;
    case PixelFormat::RGf16:    o_fmt = GL_RG; o_type = GL_HALF_FLOAT; return;
    case PixelFormat::Rf16:     o_fmt = GL_RED; o_type = GL_HALF_FLOAT; return;

    case PixelFormat::RGBAf32:  o_fmt = GL_RGBA; o_type = GL_FLOAT; return;
    case PixelFormat::RGf32:    o_fmt = GL_RG; o_type = GL_FLOAT; return;
    case PixelFormat::Rf32:     o_fmt = GL_RED; o_type = GL_FLOAT; return;

    case PixelFormat::RGBAi32:  o_fmt = GL_RGBA_INTEGER; o_type = GL_INT; return;
    case PixelFormat::RGi32:    o_fmt = GL_RG_INTEGER; o_type = GL_INT; return;
    case PixelFormat::Ri32:     o_fmt = GL_RED_INTEGER; o_type = GL_INT; return;
    default: break;
    }
}

void GraphicsDeviceOpenGL::sync()
{
    glFinish();
}

bool GraphicsDeviceOpenGL::readTexture(void *o_buf, size_t, void *tex, int, int, PixelFormat format)
{
    GLenum internal_format = 0;
    GLenum internal_type = 0;
    GetInternalFormatOpenGL(format, internal_format, internal_type);

    //// glGetTextureImage() is available only OpenGL 4.5 or later...
    // glGetTextureImage((GLuint)(size_t)tex, 0, internal_format, internal_type, bufsize, o_buf);

    sync();
    glBindTexture(GL_TEXTURE_2D, (GLuint)(size_t)tex);
    glGetTexImage(GL_TEXTURE_2D, 0, internal_format, internal_type, o_buf);
    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

bool GraphicsDeviceOpenGL::writeTexture(void *o_tex, int width, int height, PixelFormat format, const void *buf, size_t)
{
    GLenum internal_format = 0;
    GLenum internal_type = 0;
    GetInternalFormatOpenGL(format, internal_format, internal_type);

    //// glTextureSubImage2D() is available only OpenGL 4.5 or later...
    // glTextureSubImage2D((GLuint)(size_t)o_tex, 0, 0, 0, width, height, internal_format, internal_type, buf);

    glBindTexture(GL_TEXTURE_2D, (GLuint)(size_t)o_tex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, internal_format, internal_type, buf);
    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

bool GraphicsDeviceOpenGL::readBuffer(void *dst, const void *src_buf, size_t srcsize)
{
    return false;
}

bool GraphicsDeviceOpenGL::writeBuffer(void *dst_buf, const void *src, size_t srcsize)
{
    return false;
}
