#include "pch.h"

#if _WIN32
    #include <gl/GL.h>
    #pragma comment(lib, "opengl32.lib")
#else
    #include <OpenGL/gl.h>
    #include <OpenGL/glext.h>
#endif
#include "mpFoundation.h"
#include "GraphicsDevice.h"


class mpGraphicsDeviceOpenGL : public mpGraphicsDevice
{
public:
    mpGraphicsDeviceOpenGL();
    ~mpGraphicsDeviceOpenGL();
    virtual void updateDataTexture(void *tex, int width, int height, const void *data, size_t data_size);

private:
};

mpGraphicsDevice* mpCreateGraphicsDeviceOpenGL(void *device)
{
    return new mpGraphicsDeviceOpenGL();
}

mpGraphicsDeviceOpenGL::mpGraphicsDeviceOpenGL()
{
}

mpGraphicsDeviceOpenGL::~mpGraphicsDeviceOpenGL()
{
}

void mpGraphicsDeviceOpenGL::updateDataTexture(void *tex, int width, int height, const void *data, size_t data_size)
{
    const size_t num_texels = data_size / 16;
    ivec2 pos = ivec2(0, 0);
    ivec2 size = ivec2(width, ceildiv((int)num_texels, width));

    glBindTexture(GL_TEXTURE_2D, (GLuint)(size_t)tex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, pos.x, pos.y, size.x, size.y, GL_RGBA, GL_FLOAT, data);
    glBindTexture(GL_TEXTURE_2D, 0);
}
