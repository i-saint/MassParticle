#include "pch.h"
#include "UnityPluginInterface.h"

#if SUPPORT_OPENGL
#if UNITY_WIN
#include <windows.h>
#include <gl/GL.h>
#else
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#endif

#include <cstdio>
#include "mpTypes.h"
#include "mpCore_ispc.h"
#include "MassParticle.h"

class mpRendererOpenGL : public mpRenderer
{
public:
    mpRendererOpenGL();
    ~mpRendererOpenGL();
    virtual void updateDataTexture(void *tex, int width, int height, const void *data, size_t data_size);

private:
};

mpRenderer* mpCreateRendererOpenGL(void *device)
{
    return new mpRendererOpenGL();
}

mpRendererOpenGL::mpRendererOpenGL()
{
}

mpRendererOpenGL::~mpRendererOpenGL()
{
}

void mpRendererOpenGL::updateDataTexture(void *tex, int width, int height, const void *data, size_t data_size)
{
    const size_t num_texels = data_size / 16;
    ivec2 pos = ivec2(0, 0);
    ivec2 size = ivec2(width, ceildiv((int)num_texels, width));

    glBindTexture(GL_TEXTURE_2D, (GLuint)(size_t)tex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, pos.x, pos.y, size.x, size.y, GL_RGBA, GL_FLOAT, data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

#endif // SUPPORT_OPENGL
