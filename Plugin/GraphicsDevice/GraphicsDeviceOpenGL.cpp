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
    DeviceType getDeviceType() override;
    void sync() override;

    Error readTexture(void *o_buf, size_t bufsize, void *tex, int width, int height, TextureFormat format) override;
    Error writeTexture(void *o_tex, int width, int height, TextureFormat format, const void *buf, size_t bufsize) override;

    Error readBuffer(void *dst, const void *src_buf, size_t srcsize) override;
    Error writeBuffer(void *dst_buf, const void *src, size_t srcsize) override;
};


GraphicsDevice* CreateGraphicsDeviceOpenGL()
{
    return new GraphicsDeviceOpenGL();
}


void* GraphicsDeviceOpenGL::getDevicePtr() { return nullptr; }
GraphicsDevice::DeviceType GraphicsDeviceOpenGL::getDeviceType() { return DeviceType::OpenGL; }

GraphicsDeviceOpenGL::GraphicsDeviceOpenGL()
{
//    glewInit();
}

GraphicsDeviceOpenGL::~GraphicsDeviceOpenGL()
{
}


static void GetInternalFormatOpenGL(GraphicsDevice::TextureFormat format, GLenum &o_fmt, GLenum &o_type)
{
    using TextureFormat = GraphicsDevice::TextureFormat;
    switch (format)
    {
    case TextureFormat::RGBAu8:   o_fmt = GL_RGBA; o_type = GL_UNSIGNED_BYTE; return;

    case TextureFormat::RGBAf16:  o_fmt = GL_RGBA; o_type = GL_HALF_FLOAT; return;
    case TextureFormat::RGf16:    o_fmt = GL_RG; o_type = GL_HALF_FLOAT; return;
    case TextureFormat::Rf16:     o_fmt = GL_RED; o_type = GL_HALF_FLOAT; return;

    case TextureFormat::RGBAf32:  o_fmt = GL_RGBA; o_type = GL_FLOAT; return;
    case TextureFormat::RGf32:    o_fmt = GL_RG; o_type = GL_FLOAT; return;
    case TextureFormat::Rf32:     o_fmt = GL_RED; o_type = GL_FLOAT; return;

    case TextureFormat::RGBAi32:  o_fmt = GL_RGBA_INTEGER; o_type = GL_INT; return;
    case TextureFormat::RGi32:    o_fmt = GL_RG_INTEGER; o_type = GL_INT; return;
    case TextureFormat::Ri32:     o_fmt = GL_RED_INTEGER; o_type = GL_INT; return;
    default: break;
    }
}

void GraphicsDeviceOpenGL::sync()
{
    glFinish();
}

GraphicsDevice::Error GraphicsDeviceOpenGL::readTexture(void *o_buf, size_t, void *tex, int, int, TextureFormat format)
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
    return Error::OK;
}

GraphicsDevice::Error GraphicsDeviceOpenGL::writeTexture(void *o_tex, int width, int height, TextureFormat format, const void *buf, size_t)
{
    GLenum internal_format = 0;
    GLenum internal_type = 0;
    GetInternalFormatOpenGL(format, internal_format, internal_type);

    //// glTextureSubImage2D() is available only OpenGL 4.5 or later...
    // glTextureSubImage2D((GLuint)(size_t)o_tex, 0, 0, 0, width, height, internal_format, internal_type, buf);

    glBindTexture(GL_TEXTURE_2D, (GLuint)(size_t)o_tex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, internal_format, internal_type, buf);
    glBindTexture(GL_TEXTURE_2D, 0);
    return Error::OK;
}

GraphicsDevice::Error GraphicsDeviceOpenGL::readBuffer(void *dst, const void *src_buf, size_t srcsize)
{
    return Error::NotImplemented;
}

GraphicsDevice::Error GraphicsDeviceOpenGL::writeBuffer(void *dst_buf, const void *src, size_t srcsize)
{
    return Error::NotImplemented;
}
