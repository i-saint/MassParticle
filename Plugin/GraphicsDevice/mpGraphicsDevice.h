#ifndef mpGraphicsDevice_h
#define mpGraphicsDevice_h

class mpGraphicsDevice
{
public:
    virtual ~mpGraphicsDevice() {}
    virtual void updateDataTexture(void *tex, int width, int height, const void *data, size_t data_size) = 0;
};
mpGraphicsDevice* mpGetGraphicsDevice();

const int mpDataTextureWidth = 3072;
const int mpDataTextureHeight = 256;
const int mpTexelsEachParticle = 3;
const int mpParticlesEachLine = mpDataTextureWidth / mpTexelsEachParticle;

#endif // mpGraphicsDevice_h
