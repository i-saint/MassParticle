#include "UnityPluginInterface.h"

#if SUPPORT_OPENGL
#include <cstdio>
#include "mpTypes.h"
#include "mpCore_ispc.h"
#include "MassParticle.h"

class mpRendererOpenGL : public mpRenderer
{
public:
	mpRendererOpenGL();
	~mpRendererOpenGL();
	virtual void render(mpWorld &world);
	virtual void reloadShader();
	virtual void updateDataTexture(void *tex, const void *data, size_t data_size);

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

void mpRendererOpenGL::render(mpWorld &world)
{
	// I'll write this when I really need it
}

void mpRendererOpenGL::reloadShader()
{
}

void mpRendererOpenGL::updateDataTexture(void *tex, const void *data, size_t data_size)
{
	const int num_texels = data_size / 16;
	ivec2 pos = ivec2(0, 0);
	ivec2 size = ivec2(mpDataTextureWidth, ceildiv(num_texels, mpDataTextureWidth));

	glBindTexture(GL_TEXTURE_2D, (GLuint)(size_t)tex);
	glTexSubImage2D(GL_TEXTURE_2D, 0, pos.x, pos.y, size.x, size.y, GL_RGBA, GL_FLOAT, data);
	glBindTexture(GL_TEXTURE_2D, 0);
}

#endif // SUPPORT_OPENGL
